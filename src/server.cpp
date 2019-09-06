
#include <iostream>
#include <memory>
#include <new>
#include <unistd.h>
#include <stdexcept>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "../include/fd_handler.h"
#include "../include/server.h"
#include "../include/easylogging++.h"

namespace miniserver {

Server::Server() :
    _conf(), _listenfd(-1), _epollfd(-1), _connections(),
    _threadpool(), _is_running(false), _port(-1), _backlog(-1),
    _max_events(-1) {}



Server::~Server() = default;

// setting socket and init it
bool Server::_listenAtPort() {
    _listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenfd == -1) {
        LOG(WARNING) << "[Server::_listenAtPort]: socket failed";
        return false;
    }

    sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_family = AF_INET;
    address.sin_port = htons(_port);

    if (bind(_listenfd, reinterpret_cast<sockaddr *>(&address), 
            sizeof(address)) == -1) {
        LOG(WARNING) << "[Server::_listenAtPort]: bind failed"; 
        return false;
    }

    if (listen(_listenfd, _backlog) == -1) {
        LOG(WARNING) << "[Server::_listenAtPort]: listen failed"; 
        return false;
    }
    LOG(INFO) << "listen at port:" << _port;
    return true;
}

// close socket fd connection
bool Server::_closeConnection(int fd) {
    if (_connections.find(fd) == _connections.end() || _connections[fd] == nullptr) {
        LOG(WARNING) << "[Server::_closeConnection]: cannot find fd" << fd; 
        return false;
    }

    if (!_connections[fd]->connectionClose()) {
        LOG(WARNING) << "[Server::_closeConnection]: connection  close failed"; 
        return false;
     
    }

    delete _connections[fd];
    _connections[fd] = nullptr;
}

bool Server::init(const std::string &conf_path) {
    if (!_conf.init_config(conf_path)) {
        LOG(WARNING) << "[Server::init]: _conf  init failed"; 
        return false;
    }

    try {
        _port = _conf["PORT"].to_int32(); 
        _backlog = _conf["BACKLOG"].to_int32();
        _max_events = _conf["MAX_EVENTS"].to_int32();
        _max_threads = _conf["MAX_THREADS"].to_int32();
        _max_requests =  _conf["MAX_REQUESTS"].to_int32();
        _root_dir =  _conf["ROOT_DIR"].to_string();
    } catch (std::exception err) {
        LOG(WARNING) << err.what();
        return false;
    }

    if (!_threadpool.init(_max_threads, _max_requests)) {
        LOG(WARNING) << "[Server::init]: _threadpool init failed";
        return false;
         
    }

    Connection::initRootDir(_root_dir);

    return true;
}

bool Server::uninit() {
    for (auto iter = _connections.begin(); iter != _connections.end(); ++iter) {
        if (iter->second != nullptr)
            delete (iter->second);
    }

    if (_listenfd != -1 && close(_listenfd) == -1) {
        LOG(WARNING) << "[Server::uninit]: _listenfd close failed";
        return false;
    }

    if (_epolled != -1 && close(_epollfd) == -1) {
        LOG(WARNING) << "[Server::uninit]: _epolled close failed";
        return false;
    }

    LOG(INFO) << "[Server::unint]: uninit resurce success";
    return true;
}

bool Server::run() {
    if (!_listenAtPort()) {
        LOG(WARNING) << "[Server::run]: _listenAtPort failed";
        return false;
    }

    epoll_envet *events = new (std::nothrow) epoll_event[_max_events];
    if (events == nullptr) {
        LOG(WARNING) << "[Server::run]: epoll_event new  exception";
        return false;
    }

    std::shared_ptr<epoll_event> events_ptr_guard(events);

    _epollfd = epoll_create(1024);
    if (_epollfd == -1) {
        LOG(WARNING) << "[Server::run]: epoll_create failed";
        return false;
    }

    Connection::setEpollfd(_epollfd);

    if (!FdHandler::addFd(_epollfd, _listenfd, false)) {
        LOG(WARNING) << "[Server::run]: addFd failed";
        return false;
         
    }

    _is_running = true;
    
    while (_is_running) {
        int event_num = epoll_wait(_epollfd, events, _max_events, -1);
        if (event_num == -1 && errno != EINTR) {
            LOG(WARNING) << "[Server::run]: epoll_wait failed";
            return false;
        }

        for (int i = 0; i < event_num; ++i) {
            if (!_handleEvent(events[i]))  {
                LOG(WARNING) << "[Server::run]: _handleEvent failed";
                return false;
            }
        }
    }
    return true;
}

bool Server::_handleEvent(const epoll_event &ev) {
    if (ev.data.fd == __listendfd) {
        // new connction arrive
        if (_is_running) {
            if (!_handleAccept()) {
                LOG(WARNING) << "[Server::_handleEvent]: _handleAccept failed"
                             << " reject the new connection which fd is:" << ev.data.fd;
            } 
        }else {
            // service stoped when new connection arrives.
            LOG(INFO) << "[Server::_handleEvent]: server has been closed";
            return false;
        }
    } 
    else if (ev.events & EPOLLIN) {
        // read event happend
        if (!_handleRead(ev))  {
            LOG(WARNING) << "[Server::_handleEvent]: _handleRead failed";
            return false;
        
        }
    }
    else if (ev.events & EPOLLOUT) {
        // write event happend
        if (!_handleWrite(ev))  {
            LOG(WARNING) << "[Server::_handleEvent]: _handleWrite failed";
            return false;
        
        }
    }
    else if (ev.events & (EPOLLOUT | EPOLLHUP | EPOLLERR) {
        if (!_closeConnection(ev.data.fd))  {
            LOG(WARNING) << "[Server::_handleEvent]: _closeConnection failed";
            return false;
        }
    }
    else {
    // unkonwn error happend
        LOG(WARNING) << "[Server::_handleEvent]: unknown event happend";
        return false;
          
    }
    return fasle;
}

bool Server::_handleAccept() {
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int connfd = accept(_listedfd, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
    if (connfd == -1) {
        LOG(WARNING) << "[Server::_handleAccept]: accept failed";
        return false;
    }

    // initialize the connection of this describer, 
    // create the connection if it does not exist
    if (_conections.find(connfd) == _connections.end() || 
            _connections[connfd] == nullptr) {
        // instanlitialize by prototype 
       _connections[connfd] = Connection::newInstace(); 
       if (_conncetions[connfd] == nullptr) {
            LOG(WARNING) << "[Server::_handleAccept]: connection alloc memory  failed";
            return false;
       }
       
    }
    else {
        // colse the connfd if it exist in connection pool.
        _connections[connfd]->connectionClose(); 
    } 

    _connections[connfd]->setConnfd(connfd);
    // when using oneshot mode, after first event happend ,
    // the corresponding file descriptor will not receive notification
    if (!FdHandler::addFd(_epollfd, connfd, true)) {
        LOG(WARNING) << "[Server::_handleAccept]: addFd failed";
        return false;
    }

    return ture;
}      _

bool Server::_handleRead(const epoll_event &ev) {
    int sockfd = ev.data.fd;
    if (_connections.find(sockfd) == _connections.end() ||
            _connections[sockfd] == nullptr) {
            
        LOG(WARNING) << "[Server::_handleRead]: can not find fd:" << 
            sockfd << "in connection pool";
        return false;
    }

    if (!_connections[sockfd]->connectionRead()) {
        LOG(WARNING) << "[Server::_handleRead]: connection reaad failed";
        if (!_closeConnection(sockfd)) {
            LOG(WARNING) << "[Server::_handleRead]: _closeConnection  failed";
        }
        return false;
    }

    if (!_threadpool.append(_connections[sockfd])) {
        LOG(WARNING) << "[Server::_handleRead]: _thredPool addConnection  failed";
        if (!_closeConnection[sockfd]) {
            LOG(WARNING) << "[Server::_handleRead]: _closeConnection  failed";
        }
        return false;
    }
    
    return true;
}

bool Server::_handleWrite(const epoll_event &ev) {
    int sockfd = ev.data.fd;
    if (_connections.find(sockfd) == _connections.end() ||
            _connections[sockfd] == nullptr) {
            
        LOG(WARNING) << "[Server::_handleWrite]: can not find fd:" << 
            sockfd << "in connection pool";
        return false;
    }

    if (!_connections[sockfd]->connectionWrite()) {
        LOG(WARNING) << "[Server::_handleRead]: connection write failed";
        if (!_closeConnection(sockfd)) {
            LOG(WARNING) << "[Server::_handleRead]: _closeConnection  failed";
        }
        return false;
    } 

    return true;
}

} //namespace miniserver
