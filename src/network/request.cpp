#include "request.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include "esp_log.h"

static const char *TAG = "HTTP_REQUEST";

HttpRequest::HttpRequest(const char* server, int port) {
    snprintf(serverUrl, sizeof(serverUrl), "http://%s:%d", server, port);
}

HttpResponse HttpRequest::send(HttpMethod method, const char* endpoint, const char* payload, const char* contentType) {
    HttpResponse response = {-1, "", false};
    
    if (WiFi.status() != WL_CONNECTED) {
        ESP_LOGE(TAG, "WiFi not connected");
        return response;
    }
    
    HTTPClient http;
    char url[128];
    snprintf(url, sizeof(url), "%s%s", serverUrl, endpoint);
    
    ESP_LOGI(TAG, "Sending %s request to %s", 
             method == HttpMethod::GET ? "GET" :
             method == HttpMethod::POST ? "POST" :
             method == HttpMethod::PUT ? "PUT" : "DELETE", url);
    
    http.begin(url);
    
    if (contentType) {
        http.addHeader("Content-Type", contentType);
    }
    
    int httpCode;
    switch (method) {
        case HttpMethod::GET:
            httpCode = http.GET();
            break;
        case HttpMethod::POST:
            httpCode = http.POST(payload ? payload : "");
            break;
        case HttpMethod::PUT:
            httpCode = http.PUT(payload ? payload : "");
            break;
        case HttpMethod::DELETE:
            httpCode = http.sendRequest("DELETE", payload ? payload : "");
            break;
        default:
            httpCode = -1;
    }
    
    response.statusCode = httpCode;
    
    if (httpCode > 0) {
        response.success = (httpCode >= 200 && httpCode < 300);
        response.body = http.getString();
        ESP_LOGI(TAG, "HTTP response: %d", httpCode);
    } else {
        ESP_LOGE(TAG, "HTTP request failed, error: %s", http.errorToString(httpCode).c_str());
    }
    
    http.end();
    return response;
}

HttpResponse HttpRequest::sendRequest(HttpMethod method, const char* endpoint) {
    return send(method, endpoint, nullptr, nullptr);
}

HttpResponse HttpRequest::sendRequest(HttpMethod method, const char* endpoint, const char* payload) {
    return send(method, endpoint, payload, "application/json");
}

HttpResponse HttpRequest::sendRequest(HttpMethod method, const char* endpoint, const char* payload, const char* contentType) {
    return send(method, endpoint, payload, contentType);
}

HttpResponse HttpRequest::get(const char* endpoint) {
    return send(HttpMethod::GET, endpoint, nullptr, nullptr);
}

HttpResponse HttpRequest::post(const char* endpoint, const char* payload) {
    return send(HttpMethod::POST, endpoint, payload, "application/json");
}

HttpResponse HttpRequest::post(const char* endpoint, const char* payload, const char* contentType) {
    return send(HttpMethod::POST, endpoint, payload, contentType);
}

HttpResponse HttpRequest::put(const char* endpoint, const char* payload) {
    return send(HttpMethod::PUT, endpoint, payload, "application/json");
}

HttpResponse HttpRequest::del(const char* endpoint) {
    return send(HttpMethod::DELETE, endpoint, nullptr, nullptr);
}
