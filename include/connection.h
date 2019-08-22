#ifndef MINISERVER_CONNECTION_H 
#define  MINISERVER_CONNECTION_H 

#include <string>
#include "buffer.h"

namespace miniserver{

class Connection {
public:
    Connection();
    virtual ~Connection();

    // for prototype
    static Connection* newInstace();
    static void initRootDir(const std::string &root_dir);

    static int getEpollfd();
    static void setEpollfd();
    
    bool connectionClose();
    bool connectionRead();
    bool connectionWrite();

    // for function process in threadpoll
    virtual void process() = 0;

    void setConnfd(int connfd);
    int getConnfd();

protected:
    // for protoatype
    static void _addPrototype(Connection *conn);
    static std::string& _getRootDir();

   
    virtual Connection* _clone() = 0 // for prototype, return instance of subclass
    virtual bool _clear() = 0; // pass msg for subclass to process after close connectoin. 
    bool _processDone();

    bool _dumpRead(Buffer *other); // read/write buffter to other buffer
    bool _dumpWrite(Buffer *other);

    bool _dumpToRead(Buffer &other); // other buffer to read/write buffer
    bool _dumpToWrite(Buffer &other);
    
private:
    static Connection* _prototype;
    static int _epollfd;
    static std:string _root_dir;

    Buffer _read_buffer;
    Buffer _write_buffer;
    int _connfd;

    void _reset();

};


}

#endif
