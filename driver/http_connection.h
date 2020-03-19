#pragma once

#include "long_running_connection.h"
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPRequest.h>
#include <memory>
#include <string>

class HttpConnection : public LongRunningConnection {
public:
    HttpConnection(Poco::Net::HTTPRequest *request, Poco::Net::HTTPClientSession *session);

    virtual void send(std::string query);

    virtual void checkError();

    virtual void close();

    virtual bool isClosed() const;
private:
    Poco::Net::HTTPResponse response;
    std::unique_ptr <Poco::Net::HTTPRequest> request;
    std::unique_ptr <Poco::Net::HTTPClientSession> session;
    bool closed = false;
    std::ostream * out = nullptr;
    std::istream * in = nullptr;
    std::string error;
    bool errorReceived = false;
	bool firstQuery = true;
	std::size_t valuesPosition = std::string::npos;
};

