#ifndef MINISERVER_HTTP_MESSAGE_H
#define MINISERVER_HTTP_MESSAGE_H

#include <string>
#include <map>
#include <stdint.h>
#include "common_type.h"

namespace miniserver{

class HttpMessage {
public:
    HttpMessage();
    virtual ~HttpMessage();

    HttpMessage(const HttpMessage &other);
    HttpMessage& operator=(const HttpMessage &rhs);

    HttpMessage(HttpMessage &&other);
    HttpMessage& operator=(HttpMessage &&rhs);
    
    void clear();
    
    // http mesasge geter
    const std::map<std::string, std::string>& getHeader() const;
    char *getBody() const;
    uint64_t getBodyLen() const;

    // setter
    void addHeader(const std::string &key, const std::string &val);
    bool setBody(char *body_msg);
    void setBodyLen(uint64_t body_len);

private:
    std::map<std::string, std::string> _headers;
    char *_body;
    uint64_t _body_len;
    
};

//==============================================================================
//          Class Http Request
//==============================================================================

class HttpRequest : public HttpMessage {
public:
    HttpRequest();
    ~HttpRequest();
    
    HttpRequest(const HttpRequest &other);
    HttpRequest& operator=(const HttpRequest &rhs);

    HttpRequest(HttpRequest &&other);
    HttpRequest& operator=(HttpRequest &&rhs);
    
    void clear();
    
    // getter
    const std::string& getMethod() const;
    const std::string& getUrl() const;
    const std::string& getVersion() const;

    // setter
    void setMethod(const std::string &method);
    void setUrl(const std::string &url);
    void setVersion(const std::string &version);

private:
    std::string _method;
    std::string _url;
    std::string _version;
    
};

//==============================================================================
//          Class Http Response
//==============================================================================

class HttpResponse : public HttpMessage {
public:
    HttpResponse();
    ~HttpResponse();
    
    HttpResponse(const HttpResponse &other);
    HttpResponse& operator=(const HttpResponse &rhs);

    HttpResponse(HttpResponse &&other);
    HttpResponse& operator=(HttpResponse &&rhs);
    
    void clear();
    
    // getter
    const std::string& getStatus() const;
    const std::string& getInfo() const;
    const std::string& getVersion() const;

    // setter
    void setStatus(const std::string &status);
    void setInfo(const std::string &info);
    void setVersion(const std::string &version);

private:
    std::string _status;
    std::string _info;
    std::string _version;
};

}
#endif
