#include "../include/fd_handler.h"

namespace miniserver {

int FdHandler::setNonblock(int fd) {
int old_fd = fcntl(fd, F_GETFL); 
    if (old_fd == -1) {
        LOG(WARNING) << "[FdHandler::setNonblock]: set fcntl falied";
        return -1;
    }
    return old_fd;
}

bool FdHandler::addFd(int epollfd, int fd, bool one_shot) {
    epoll_event event; 
    event.data.fd = fd;
   
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    if (one_shot) {
       event.events |= EPOLLONESHOT; 
    }

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event) == -1) {
        LOG(WARNING) << "[FdHandler::addFd]:epoll_ctl falied";
        return false;
    }
    if (setNonblock(fd) == -1) {
        LOG(WARNING) << "[FdHandler::addFd]: setNonblock falied";
        return false;
    }

    return true;
}

bool FdHandler::removeFd(int epollfd, int fd) {
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, 0) == -1) {
        LOG(WARNING) << "[FdHandler::addFd]: setNonblock falied";
        return false;
    }

    if (close(fd) == -1) {
        LOG(WARNING) << "[FdHandler::addFd]: close fd falied";
        return false;
    }
    return true;
}

bool FdHandler::modFd(int epollfd, int fd, int ev) {
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event) == -1) {
        LOG(WARNING) << "[FdHandler::modFd]: epoll_ctl falied";
        return false;
    }
    return true;
}
    
}// namespace miniserver
