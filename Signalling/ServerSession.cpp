#include "ServerSession.h"

#include "RtspSession/StatusCode.h"


bool ServerSession::handleOptionsRequest(
    const rtsp::Request& request) noexcept
{
    rtsp::Response response;
    response.protocol = rtsp::Protocol::RTSP_1_0;
    response.cseq = request.cseq;
    response.statusCode = rtsp::OK;
    response.reasonPhrase = "OK";

    response.headerFields.emplace("Public", "DESCRIBE, SETUP, PLAY, TEARDOWN");

    sendResponse(&response);

    return true;
}

bool ServerSession::handleDescribeRequest(const rtsp::Request& request) noexcept
{
    return false;
}

bool ServerSession::handleSetupRequest(const rtsp::Request&) noexcept
{
    return false;
}

bool ServerSession::handlePlayRequest(const rtsp::Request&) noexcept
{
    return false;
}

bool ServerSession::handleTeardownRequest(const rtsp::Request&) noexcept
{
    return false;
}