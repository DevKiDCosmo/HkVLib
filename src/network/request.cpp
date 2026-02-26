#include "request.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include "esp_log.h"
#include "../serial/log.h"
#include "../config/config.h"

static const char *TAG = "HTTP_REQUEST";

namespace
{
    String resolveServer(const String &fallbackServer)
    {
        if (Configuration::hasStmt("server"))
        {
            const String &configuredServer = Configuration::getExprForStmt("server");
            if (!configuredServer.isEmpty())
            {
                return configuredServer;
            }
        }

        return fallbackServer;
    }

    int resolvePort(int fallbackPort)
    {
        if (Configuration::hasStmt("port"))
        {
            const String &configuredPort = Configuration::getExprForStmt("port");
            if (!configuredPort.isEmpty())
            {
                const int parsedPort = configuredPort.toInt();
                if (parsedPort > 0)
                {
                    return parsedPort;
                }

                Log::sys_warning(TAG, "Invalid configured port: " + configuredPort + ", using fallback");
            }
        }

        return fallbackPort;
    }
}

HttpRequest::HttpRequest()
{
    const String server = resolveServer(String(SERVER));
    const int port = resolvePort(PORT);
    serverUrl = "http://" + server + ":" + String(port);
}

HttpRequest::HttpRequest(const String &server, int port)
{
    const String resolvedServer = resolveServer(server);
    const int resolvedPort = resolvePort(port);
    serverUrl = "http://" + resolvedServer + ":" + String(resolvedPort);
}

HttpResponse HttpRequest::send(HttpMethod method, const String &endpoint, const String &payload, const String &contentType)
{
    HttpResponse response = {-1, "", false};

    if (WiFi.status() != WL_CONNECTED)
    {
        Log::sys_error(TAG, "WiFi not connected");
        return response;
    }

    HTTPClient http;
    String url = serverUrl + endpoint;

    const char *methodStr = method == HttpMethod::GET ? "GET" : method == HttpMethod::POST ? "POST"
                                                            : method == HttpMethod::PUT    ? "PUT"
                                                                                           : "DELETE";
    Log::sys_info(TAG, "Sending " + String(methodStr) + " request to " + url);

    http.begin(url);

    if (contentType.length() > 0)
    {
        http.addHeader("Content-Type", contentType);
    }

    int httpCode;
    switch (method)
    {
    case HttpMethod::GET:
        httpCode = http.GET();
        break;
    case HttpMethod::POST:
        httpCode = http.POST(payload);
        break;
    case HttpMethod::PUT:
        httpCode = http.PUT(payload);
        break;
    case HttpMethod::DELETE:
        httpCode = http.sendRequest("DELETE", payload);
        break;
    default:
        httpCode = -1;
    }

    response.statusCode = httpCode;

    if (httpCode > 0)
    {
        response.success = (httpCode >= 200 && httpCode < 300);
        response.body = http.getString();
        Log::sys_info(TAG, "HTTP response: " + String(httpCode));
    }
    else
    {
        Log::sys_error(TAG, "HTTP request failed, error: " + http.errorToString(httpCode));
    }

    http.end();
    return response;
}

HttpResponse HttpRequest::sendRequest(HttpMethod method, const String &endpoint)
{
    return send(method, endpoint, "", "");
}

HttpResponse HttpRequest::sendRequest(HttpMethod method, const String &endpoint, const String &payload)
{
    return send(method, endpoint, payload, "application/json");
}

HttpResponse HttpRequest::sendRequest(HttpMethod method, const String &endpoint, const String &payload, const String &contentType)
{
    return send(method, endpoint, payload, contentType);
}

HttpResponse HttpRequest::get(const String &endpoint)
{
    return send(HttpMethod::GET, endpoint, "", "");
}

HttpResponse HttpRequest::post(const String &endpoint, const String &payload)
{
    return send(HttpMethod::POST, endpoint, payload, "application/json");
}

HttpResponse HttpRequest::post(const String &endpoint, const String &payload, const String &contentType)
{
    return send(HttpMethod::POST, endpoint, payload, contentType);
}

HttpResponse HttpRequest::put(const String &endpoint, const String &payload)
{
    return send(HttpMethod::PUT, endpoint, payload, "application/json");
}

HttpResponse HttpRequest::del(const String &endpoint)
{
    return send(HttpMethod::DELETE, endpoint, "", "");
}
