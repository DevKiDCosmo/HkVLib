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
    HttpRequest(const String& server, int port);

    HttpResponse sendRequest(HttpMethod method, const String& endpoint);
    HttpResponse sendRequest(HttpMethod method, const String& endpoint, const String& payload);
    HttpResponse sendRequest(HttpMethod method, const String& endpoint, const String& payload, const String& contentType);

    HttpResponse get(const String& endpoint);
    HttpResponse post(const String& endpoint, const String& payload);
    HttpResponse post(const String& endpoint, const String& payload, const String& contentType);
    HttpResponse put(const String& endpoint, const String& payload);
    HttpResponse del(const String& endpoint);

private:
    String serverUrl;

    HttpResponse send(HttpMethod method, const String& endpoint, const String& payload, const String& contentType);
};

#endif
