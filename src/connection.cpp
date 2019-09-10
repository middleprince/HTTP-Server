#include <cstring>
#include <cstdio>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "../include/connection.h"
#include "../include/fd_handler.h"
#include "../include/easylogging++.h"

namespace miniserver {

Connection* Connection::_prototype = nullptr;

int Connection::_epollfd = -1;

std::string Connection::_root_dir = "";

std::string& Connection::_getRootDir() {
    return _root_dir;
}

void Connection::initRootDir(const std::string &root_dir) {
    _root_dir = root_dir;
}

void Connection::_addPrototype(Connection *conn) {
    _prototype = conn;
}

Connection* Connection::newInstace() {
    return _prototype->_clone();
}

void Connection::setEpollfd(int epollfd) {
   _epollfd = epollfd; 
}

int Connection::getEpollfd() {
    return _epollfd;
}


Connection::Connection() : _read_buffer(), _write_buffer(), _connfd(-1) {}

Connection::~Connection() = default;

void Connection::_reset() {
    _read_buffer.clear();
    _write_buffer.clear();
    if (!_clear()) {
        LOG (WARNING) << "[Connection::_reset]: _clear";
    }
}

void Connection::setConnfd(int connfd) {
    _connfd = connfd;
}

int Connection::getConnfd() {
    return _connfd;
}

bool Connection::connectionClose() {
    _read_buffer.clear();
    _write_buffer.clear();

    if (!FdHandler::removeFd(_epollfd, _connfd)) {
        LOG (WARNING) << "[Connection::connectionClose]: remove fd failed";
        return false;
    }
    
    if (!_clear()) {
        LOG (WARNING) << "[Connection::connectionClose]: _clear() failed";
        return false;
    }

    LOG (INFO) << "[Connection::connectionClose]: close connection success";
    return true;
}

bool Connection::connectionRead() {
    if (_connfd == -1) {
        LOG (WARNING) << "[Connection::connectionRead]: invalid fd";
        return false;
    }

    if (!_read_buffer.empty()) {
        _read_buffer.clear(); 
    }

    uint64_t buffer_size = 2048;
    ssize_t ret = 0;
    char buffer[buffer_size];

    while (true) {
        memset(buffer, 0, sizeof(char) * buffer_size); 
        ret = recv(_connfd, buffer, buffer_size, 0);

        if (ret == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; 
            } 
            LOG(WARNING) << "[Connection::connectionRead]: recv failed";
            return false;
        } else if (ret == 0) {
            return true; 
        }

        _read_buffer.pushBytes(buffer, ret * sizeof(char));
    }
    return true;
}

bool Connection::connectionWrite() {
    if (_write_buffer.empty()) {
        if (!FdHandler::modFd(_epollfd, _connfd, EPOLLIN)) {
            LOG(WARNING) << "[Connection::connectionWrite]: modFd failed";
            return false;
        } 
    }

    int64_t ret = 0;
    while (true) {
        if (_write_buffer.leftBytes() == 0) {
            if (!FdHandler::modFd(_epollfd, _connfd, EPOLLIN)) {
                LOG(WARNING) << "[Connection::connectionWrite]: modFd failed";
                return false;
            } 
            _reset();
            return true;
        } 

        if (_write_buffer.currentAddr() == nullptr) {
            LOG(WARNING) << "[Connection::connectionWrite]: get currentAddr failed";
            return false;
        }

        ret = send(_connfd, _write_buffer.currentAddr(), _write_buffer.leftBytes(), 0);

        if (ret == 0) {
            if (!FdHandler::modFd(_epollfd, _connfd, EPOLLIN)) {
                LOG(WARNING) << "[Connection::connectionWrite]: modFd failed";
                return false;
            }
            _reset();
            return true;
        }

        if (ret == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if (!FdHandler::modFd(_epollfd, _connfd, EPOLLIN)) {
                    LOG(WARNING) << "[Connection::connectionWrite]: modFd failed";
                    return false;
                }
                _reset();
                return true;
            } else {
                return true; 
            } 
        }
        _write_buffer.rollBytes(ret);
    }

    _reset();
    return true;
}

bool Connection::_dumpRead(Buffer *other_buffer) {
    if (other_buffer == nullptr) {
        LOG(WARNING) << "[Connecctoin::_dumpRead]: dump read_buffer destnation is void"; 
        return false;
    }
    if (!other_buffer->empty()) {
        other_buffer->clear(); 
    }

    if (!_read_buffer.dump(other_buffer)) {
        LOG(WARNING) << "[Connecctoin::_dumpRead]: dump read_buffer fialed"; 
        return false;
    }
    return true;
}

bool Connection::_dumpWrite(Buffer *other_buffer) {
    if (other_buffer == nullptr) {
        LOG(WARNING) << "[Connecctoin::_dumpRead]: dump write_buffer destnation is void"; 
        return false;
    }
    if (!other_buffer->empty()) {
        other_buffer->clear(); 
    }

    if (!_write_buffer.dump(other_buffer)) {
        LOG(WARNING) << "[Connecctoin::_dumpWrite]: dump write_buffer failed"; 
        return false;
    }
    return true;
}

bool Connection::_dumpToRead(Buffer &other_buffer) {
    if (!_read_buffer.empty()) {
        _read_buffer.clear(); 
    }
    if (!other_buffer.dump(&_read_buffer)) {
        LOG(WARNING) << "[Connecctoin::_dumpToRead]: dump to read_buffer failed"; 
        return false;
    }
    return true;
}

bool Connection::_dumpToWrite(Buffer &other_buffer) {
    if (!_write_buffer.empty()) {
        _write_buffer.clear(); 
    }
    if (!other_buffer.dump(&_write_buffer)) {
        LOG(WARNING) << "[Connecctoin::_dumpToWrite]: dump to write_buffer failed"; 
        return false;
    }
    return true;
}

bool Connection::_processDone() {
    if (!FdHandler::modFd(_epollfd, _connfd, EPOLLOUT)) {
        LOG(WARNING) << "[Connection::connectionWrite]: modFd failed";
        return false;
    }
    return true;
}


}

