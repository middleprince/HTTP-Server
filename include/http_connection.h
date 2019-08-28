#ifndef MINISERVER_HTTP_CONNECTION_H
#define MINISERVER_HTTP_CONNECTION_H

#include <map>
#include "connection.h"
#include "http_message.h"
#include "common_type.h"

namespace miniserver {

class HttpConnection : public Connection {
public:
    // copy contorl constructor for prototype.
    HttpConnection(int dummy);        
    ~HttpConnection() override;

    void  process() override;
    
protected:
    HttpConnection();
    
    Connection* _clone() override;
    bool _clear() override;
    
    static HttpConnection _http_connection;

private:
    void _parseRequest(Buffer &buffer);
    bool _parseRequestLine(const std::string &line);
    bool _parseHeader(const std::string &line);
    bool _parseBody(Buffer &buffer);
    
    bool _creatResponseStream(Buffer *buffer);
    
    std::string _creatStatus(const HttpCode &http_code);
    std::string _creatStatusInfo(const HttpCode &http_code);
    void _errorResponse(const HttpCode &http_code);
    
    HttpCode _headleRequest();
    HttpCode _handleGet();

    static void _initMime();
    
    static std::map<std::string, std::string> _mime_type;
    HttpRequest _request;
    HttpResponse _response;
    ParseStatus _parse_status;

    HttpConnection(const HttpConnection &other);
    HttpConnection& operator=(const HttpConnection &rhs);

    
};

}
#endif
