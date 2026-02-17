#ifndef REQUEST_H
#define REQUEST_H

#include <Arduino.h>

enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE
};

struct HttpResponse {
    int statusCode;
    String body;
    bool success;
};

class HttpRequest {
public:
    HttpRequest(const char* server, int port);
    
    HttpResponse sendRequest(HttpMethod method, const char* endpoint);
    HttpResponse sendRequest(HttpMethod method, const char* endpoint, const char* payload);
    HttpResponse sendRequest(HttpMethod method, const char* endpoint, const char* payload, const char* contentType);
    
    HttpResponse get(const char* endpoint);
    HttpResponse post(const char* endpoint, const char* payload);
    HttpResponse post(const char* endpoint, const char* payload, const char* contentType);
    HttpResponse put(const char* endpoint, const char* payload);
    HttpResponse del(const char* endpoint);
    
private:
    char serverUrl[64];
    
    HttpResponse send(HttpMethod method, const char* endpoint, const char* payload, const char* contentType);
};

#endif
