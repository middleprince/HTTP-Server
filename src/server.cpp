#include <iostream>
#include <memory>
#include <new>
#include <unistd.h>
#include <stdexcept>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "fd_handler.h"
#include "../include/server.h"
#include "easylogging++.h"

namespace miniserver {

Server::Server() :
    _conf(), _listenfd(-1), _epollfd(-1), _connections(),
    _threadpool(), _is_running(false), _port(-1), _back_log(-1),
    _max_events(-1) {}

}

Server::~Server() = default;

// setting socket and init it
bool Server::_listenAtPort() {
    _listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenfd == -1) {
        LOG(WARNING) << "[Server::_listenAtPort]: socket failed";
        return false;
    }

    sockaddr_in address;
    bzeros(&address, sizeof(address));
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_family = AF_INET;
    address.sin_port = htons(_port);

    if (bind(_listenfd, reinterpret_cast<socketaddr *>(&address), 
            sizeof(address)) == -1) {
        LOG(WRNING) << "[Server::_listenAtPort]: bind failed"; 
        reutrn fasle;
    }

    if (listen(_listenfd, _back_log) == -1) {
        LOG(WRNING) << "[Server::_listenAtPort]: listen failed"; 
        reutrn fasle;
    }
    LOG(INFO) << "listen at port:" << _port;
    return ture;
}

// close socket fd connection
bool Server::_closeConnection(int fd) {
    if (_conncetions.find(fd) == _connections.end() || _conncetions[fd] == nullptr) {
        LOG(WRNING) << "[Server::_closeConnection]: cannot find fd" << fd; 
        reutrn fasle;
    }

    if (!_connections[fd]->connectionClose()) {
        LOG(WRNING) << "[Server::_closeConnection]: connection  close failed"; 
        reutrn fasle;
     
    }

    delete _connections[fd];
    _connection[fd] = nullptr;
}


