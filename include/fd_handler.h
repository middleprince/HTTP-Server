#ifndef MINISERVER_FD_HANDLER_H
#define MINISERVER_FD_HANDLER_H

#include <fcntl.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include "easylogging++.h"

namespace miniserver {

class FdHandler {
    FdHandler() = default;
    ~FdHandler() = default;

    static int setNonblock(int fd);
    static bool addFd(int epollfd, int fd);
    static bool removeFd(int epollfd, int fd);
    static bool modFd(int epollfd, int fd, int ev);
};

}
#endif
