#ifndef MINISERVER_BUFFER_H 
#define  MINISERVER_BUFFER_H 

#include <vector>
#include <string> 
#include <cstdint>

#include "common_type.h"

namespace miniserver{

// @bref:buffer for reciving and sending data
// function for read and write data from/to buffer/  
// @func: getLine()
// @func: pushBytes()
//
// 
class Buffer{
public:
    Buffer();
    ~Buffer();

    Buffer(const Buffer &other);
    Buffer& operator=(const Buffer &rhs);

    Buffer(const Buffer &&other);
    Buffer& operator=(const Buffer &&rhs);

    uint64_t size() const;
    bool empty() const;
    void clear();
    bool dump(Buffer *other);  // dump _buffer to Buffer ohter

    void pushBytes(const char *buffer, uint64_t nbytes); //push n bytes start from buffer
    void rollBytes(uint64_t nbytes); // roll n cursor n bytes
    
    BufferReadStatus getLine(std::string *line);
    bool readUtilEnd(std::string *content); // read all data 
    void* currentAddr() const; 
    uint64_t letfBytes() const;

    void showContent() const;
    
private:
   uint64_t _cursor; 
   std::vector<char> _buffer;
    
};

}


#endif
