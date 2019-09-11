#include <string>
#include <fstream>
#include <new>
#include <cstring>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include "../include/easylogging++.h"
#include "../include/http_connection.h"

namespace miniserver {
    
HttpConnection HttpConnection::_http_connection;

std::map<std::string, std::string> HttpConnection::_mime_type; 

HttpConnection::HttpConnection() :
    _request(), _response(), _parse_status(ParseStatus::PARSE_REQUEST) {
        _addPrototype(this); 
    }

HttpConnection::HttpConnection(int dummy) : 
    _request(), _response(), _parse_status(ParseStatus::PARSE_REQUEST) {}

HttpConnection::~HttpConnection() = default;

Connection* HttpConnection::_clone() {
    return new HttpConnection(5);
}

void HttpConnection::_initMime() {
    std::string ext_name = "";
    std::string mime_type = "";
   
    std::stringstream ss;
    std::ifstream mime_file("../mime.txt");
    std::string line = "";

    while (std::getline(mime_file, line)) {
        ss.str(line); 
        ss >> mime_type >> ext_name;
        ss.clear();
        _mime_type[ext_name] = mime_type;
    }
}

bool HttpConnection::_clear() {
    _request.clear();
    _response.clear();
    _parse_status = ParseStatus::PARSE_REQUEST;
    return true;
}

void HttpConnection::process() {
    Buffer read_buffer;
    Buffer write_buffer;

    if (!_dumpRead(&read_buffer) || read_buffer.empty()) {
        _errorResponse(HttpCode::HTTP_INTERNAL_ERROR); 
        return ;
    }
    
    _parseRequest(read_buffer);
    if (_parse_status == ParseStatus::PARSE_ERROR) {
        _errorResponse(HttpCode::HTTP_BAD_REQUEST); 
        return ;
    }

    if (_parse_status == ParseStatus::PARSE_SUCCESS) {
        return ; 
    }

    HttpCode code = _handleRequest();
    if (code != HttpCode::HTTP_OK) {
        _errorResponse(code); 
        return ;
    }

    if (!_createResponseStream(&write_buffer)) {
        _errorResponse(HttpCode::HTTP_INTERNAL_ERROR); 
        return ;
    }

    if (!_dumpToWrite(write_buffer)) {
        _errorResponse(HttpCode::HTTP_INTERNAL_ERROR);
        return ;
    }

    if (!_processDone()) {
        LOG(WARNING) << "[HttpConnection::process]: _processDone failed";
        return ;
    }
}

std::string HttpConnection::_createStatus(const HttpCode &code) {
    switch (code) {
    case HttpCode::HTTP_OK:
       return "200"; 
    case HttpCode::HTTP_BAD_REQUEST:
       return "400";
    case HttpCode::HTTP_FORBIDDEN:
       return "403";
    case HttpCode::HTTP_NOT_FOUND:
       return "404";
    case HttpCode::HTTP_METHOD_NOT_ALLOWED:
       return "405";
    case HttpCode::HTTP_INTERNAL_ERROR:
       return "500";
    }
}

std::string HttpConnection::_createStatusInfo(const HttpCode &code) {
    switch (code) {
    case HttpCode::HTTP_OK:
       return "OK"; 
    case HttpCode::HTTP_BAD_REQUEST:
       return "Bad Request"; 
    case HttpCode::HTTP_FORBIDDEN:
       return "Forbidden"; 
    case HttpCode::HTTP_NOT_FOUND:
       return "Not Found"; 
    case HttpCode::HTTP_METHOD_NOT_ALLOWED:
       return "Method Not Allowed"; 
    case HttpCode::HTTP_INTERNAL_ERROR:
       return "Internal Server Error"; 
    }     
}

void HttpConnection::_errorResponse(const HttpCode &code) {
    Buffer buffer;
    
    // make status line 
    std::string status_line = _request.getVersion() + " " + 
        _createStatus(code) + " " + _createStatusInfo(code) + "\r\n";
    buffer.pushBytes(status_line.c_str(), status_line.length());

    char buf[1024]; 
    sprintf(buf, "<html><title>MiniServer Error</title>");
    sprintf(buf, "%s<body bgcolor=""ffffff"">\r\n", buf);
    sprintf(buf, "%s%s: %s\r\n", buf, _createStatus(code).c_str(), _createStatusInfo(code).c_str());
    sprintf(buf, "%s<hr><em>The MiniServer</em>\r\n", buf);
    
    // make header content
    std::string header_line = "";
    header_line = "Content-Type:text/html\r\n";
    buffer.pushBytes(header_line.c_str(), header_line.length());
    header_line = "\r\n";
    buffer.pushBytes(header_line.c_str(), header_line.length());
    
    //make response body
    buffer.pushBytes(buf, strlen(buf));

    if (!_dumpToWrite(buffer)) {
        LOG(FATAL) << "[HttpConnection::_errorResponse]: _dumpToWrite failed";
        return ;
    } 
    if (!_processDone()) {
        LOG(FATAL) << "[HttpConnection::_errorResponse]: _processDone failed";
        return ;
    }
}

bool HttpConnection::_createResponseStream(Buffer *buffer) {
    if (buffer == nullptr) {
        LOG(WARNING) << "[HttpConnection::_createResponseStream]: buffer nullptr"; 
            return false;
    }
    buffer->clear();

    std::string status_line = _response.getVersion() + " "  + _response.getStatus() + 
        " " + _response.getInfo() + "\r\n";
    buffer->pushBytes(status_line.c_str(), status_line.length());

    const std::map<std::string, std::string> &headers = _response.getHeaders();
    std::string header_line = "";
    for (auto iter = headers.begin(); iter != headers.end(); ++iter) {
        header_line = iter->first + ":" + iter->second + "\r\n"; 
        buffer->pushBytes(header_line.c_str(), header_line.length());
    }

    header_line = "\r\n";
    buffer->pushBytes(header_line.c_str(), header_line.length());

    if (_response.getBody() == nullptr || _response.getBodyLen() == 0) {
        LOG(WARNING) << "[HttpConnection::_createResponseStream]: response body is void";
        return false;
    }

    buffer->pushBytes(_response.gerBody(), _response.getBodyLen());

    if (mummap(_response.getBody(), _response.gerBodyLen()) == -1) {
        LOG(WARNING) << "[HttpConnection::_createResponseStream]: mummap failed"; 
        return false;
    }
    _response.setBody(nullptr);
    _response.setBodyLen(0);
    return true;
}


}

