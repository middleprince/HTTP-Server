#ifndef MINISERVER_COMMON_TYPE_H
#define MINISERVER_COMMON_TYPE_H

namespace miniserver {

// message code for buffer
enum class BufferReadStatus {
    READ_SUCCESS = 0,  // read success
    READ_END = 1, // reching the end of buffer
    READ_FIALED = -1, // read error which is can not find \n\r int buffer
    READ_ERROR = -2, // the parameter is nullptr
    READ_OUTBOUND = -3 // the cursor of the buffer is out of bound
};

// status code for pasing manipulation
enum class ParseStatus {
    PARSE_SUCCESS = 0, // parse input sucessfully
    PARSE_REQUEST = 1, // parse request line
    PARSE_HEADER = 2, //parse headers_    
    PARSE_BODY = 3, // parse body
    PARSE_ERROR = -1, // parse error
};

// http status code
enum class HttpCode {
    HTTP_OK = 200, // sucess code
    HTTP_BAD_REQUEST = 400, // request illeagal
    HTTP_FORBIDDEN = 403, //requested file is not allowed for acessing
    HTTP_NOT_FOUND = 404, //file not found
    HTTP_METHOD_NOT_ALLOWED = 405, //the method is not allow
    HTTP_INTERNAL_ERROR = 500, //server internal privoxy error
};

}

#endif
