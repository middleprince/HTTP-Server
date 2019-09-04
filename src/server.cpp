
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
    }_

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

}
