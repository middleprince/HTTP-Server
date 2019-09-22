#include <utility>
#include <algorithm>
#include "../include/http_message.h"
#include "../include/easylogging++.h"

namespace miniserver {

HttpMessage::HttpMessage() :
    _headers(), _body(nullptr), _body_len(0) {}

HttpMessage::~HttpMessage() {
    if (_body != nullptr) {
        delete[] _body; 
        _body = nullptr;
    }
    _body_len = 0;
}

HttpMessage::HttpMessage(const HttpMessage &other) :
    _headers(other._headers), _body(other._body), _body_len(other._body_len) {}

HttpMessage& HttpMessage::operator=(const HttpMessage &rhs) {
    if (this != &rhs) {
        _headers = rhs._headers; 
        _body = rhs._body;
        _body_len = rhs._body_len;
    }
    return *this;
} 

HttpMessage::HttpMessage(HttpMessage &&other) : 
    _headers(std::move(other._headers)),
    _body(std::move(other._body)) {
    other._body = nullptr;    
    other._body_len = 0;
}

HttpMessage & HttpMessage::operator=(HttpMessage &&other) {
    if (this != &other) {
        _headers = std::move(other._headers); 
        _body = other._body;
        _body_len = other._body_len;

        other._body = nullptr;
        other._body_len = 0;
    }
    return *this;
}

void HttpMessage::clear() {
    _headers.clear();
    _body_len = 0;
    if (_body != nullptr) {
        delete[] _body; 
    }
}

const std::map<std::string, std::string>& 
HttpMessage::getHeader() const {
    return _headers;
}

char* HttpMessage::getBody() const {
    return _body;
}

uint64_t HttpMessage::getBodyLen() const {
    return _body_len;
}

void HttpMessage::addHeader(const std::string &key, const std::string &value) {
    _headers.insert(std::make_pair(key, value));
}

bool HttpMessage::setBody(char *body) {
    _body = body;
    return true;
}

void HttpMessage::setBodyLen(uint64_t body_len) {
    _body_len = body_len;
}



//==============================================================================
//          HttpRequest
//==============================================================================

HttpRequest::HttpRequest() : HttpMessage(), 
    _method(" "), _url(" "), _version("") {}

HttpRequest::~HttpRequest() = default;

HttpRequest::HttpRequest(const HttpRequest &other) : 
    HttpMessage(other), 
    _method(other._method),
    _url(other._url),
    _version(other._version) {}

HttpRequest& HttpRequest::operator=(const HttpRequest &rhs) {
    if (this != &rhs) {
        HttpMessage::operator=(rhs);
      _method = rhs._method;  
      _url = rhs._url;
      _version = rhs._version;
    }
    return *this;
}

void HttpRequest::clear() {
    HttpMessage::clear();
    _method.clear();
    _url.clear();
    _version.clear();
}

const std::string& HttpRequest::getMethod() const {
    return _method;
}

const std::string& HttpRequest::getUrl() const {
    return _url;
}
const std::string& HttpRequest::getVersion() const {
    return _version;
}

void HttpRequest::setMethod(const std::string &method) {
    _method = method;
}

void HttpRequest::setUrl(const std::string &url) {
    _url = url;
}

void HttpRequest::setVersion(const std::string &version) {
    _version = version;
}


//==============================================================================
//          HttpRespose src
//==============================================================================

HttpResponse::HttpResponse() : HttpMessage(), 
    _status(" "), _info(" "), _version("") {}

HttpResponse::~HttpResponse() = default;

HttpResponse::HttpResponse(const HttpResponse &other) : 
    HttpMessage(other), 
    _status(other._status),
    _info(other._info),
    _version(other._version) {}

HttpResponse& HttpResponse::operator=(const HttpResponse &rhs) {
    if (this != &rhs) {
        HttpMessage::operator=(rhs);
      _status = rhs._status;  
      _info = rhs._info;
      _version = rhs._version;
    }
    return *this;
}

void HttpResponse::clear() {
    HttpMessage::clear();
    _status.clear();
    _info.clear();
    _version.clear();
}

const std::string& HttpResponse::getStatus() const {
    return _status;
}

const std::string& HttpResponse::getInfo() const {
    return _info;
}
const std::string& HttpResponse::getVersion() const {
    return _version;
}

void HttpResponse::setStatus(const std::string &method) {
    _status = method;
}

void HttpResponse::setInfo(const std::string &url) {
    _info = url;
}

void HttpResponse::setVersion(const std::string &version) {
    _version = version;
}





} // namespace miniserver

