// 服务器最低层模块 
// 
//

#ifndef MINISERVER_SERVER_H
#define MINISERVER_SERVER_H

#include <string>
#include <map>
#include <sys/epoll.h>
#include "configure.h"
#include "threadpool.h"
#include "connection.h"

namespace miniserver {

class Server {
public:
    Server();
    ~Server();

    bool startServer();
    bool init(const std::string &conf_path);
    bool uninit();
    
    void askToQuit();
private:
    bool _listenAtPort(); 
    bool _closeConnection(int fd);

    bool _handleEvent(const epoll_event &ev);
    bool _handleAccept();
    bool _handleRead(const epoll_event &ev);
    bool _handleWrite(const epoll_event &ev);

    Configure _conf; // configure tool class
    int _listenfd;  // file descriptor
    int _epollfd; // epoll descriptor
    std::map<int, Connection*> _connections; // map for every fd and connection 
    ThreadPool<Connection> _threadpool; // thread pool for http parse
    bool _is_running; //is server running

    // server configurations
    int _port;
    int _backlog;
    int _max_events;
    int _max_threads;
    int _max_requests;
    std::string _root_dir;

    // delete copy and assign
    Server(const Server &other);
    Server& operator=(const Server &rhs);
    
};

}

#endif
