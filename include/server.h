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

    
private:
    

};

}

#endif
