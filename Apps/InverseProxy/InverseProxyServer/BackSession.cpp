#include "BackSession.h"

#include "RtspSession/StatusCode.h"


struct BackSession::Private
{
    Private(BackSession* owner, ForwardContext*);

    BackSession *const owner;
    ForwardContext *const forwardContext;

    std::string clientName;

    std::map<std::string, std::string> sessionId2frontSessionId;
};

BackSession::Private::Private(
    BackSession* owner, ForwardContext* forwardContext) :
    owner(owner), forwardContext(forwardContext)
{
}


BackSession::BackSession(
    ForwardContext* forwardContext,
    const std::function<void (const rtsp::Request*)>& sendRequest,
    const std::function<void (const rtsp::Response*)>& sendResponse) noexcept :
    rtsp::Session(sendRequest, sendResponse),
    _p(new Private(this, forwardContext))
{
}

BackSession::~BackSession()
{
    _p->forwardContext->removeBackSession(_p->clientName, this);
}

bool BackSession::handleRequest(std::unique_ptr<rtsp::Request>& requestPtr) noexcept
{
    if(rtsp::Method::SET_PARAMETER != requestPtr->method && _p->clientName.empty())
        return false;

    return rtsp::Session::handleRequest(requestPtr);
}

bool BackSession::handleSetParameterRequest(std::unique_ptr<rtsp::Request>& requestPtr) noexcept
{
    if(RequestContentType(*requestPtr) != "text/parameters")
        return false;

    const std::string& parameters = requestPtr->body;

    const std::string::size_type delimiterPos = parameters.find(":");
    if(delimiterPos == std::string::npos || 0 == delimiterPos)
        return false;

    const std::string::size_type lineEndPos = parameters.find("\r\n", delimiterPos + 1);
    if(lineEndPos == std::string::npos)
        return false;

    const std::string name = parameters.substr(0, delimiterPos - 0);

    const std::string value =
        parameters.substr(delimiterPos + 1, lineEndPos - (delimiterPos + 1));

    if(name != "name")
        return false;

    if(!_p->clientName.empty())
        return false;

    if(!_p->forwardContext->registerBackSession(value, this))
        return false;

    _p->clientName = value;

    return true;
}

bool BackSession::handleSetupRequest(std::unique_ptr<rtsp::Request>& requestPtr) noexcept
{
    return _p->forwardContext->forwardToFrontSession(this, *requestPtr);
}

rtsp::CSeq BackSession::forward(std::unique_ptr<rtsp::Request>& requestPtr)
{
    rtsp::Request* request = createRequest(requestPtr->method, requestPtr->uri);

    request->headerFields.swap(requestPtr->headerFields);
    request->body.swap(requestPtr->body);
    requestPtr.reset();

    sendRequest(*request);

    return request->cseq;
}

void BackSession::forward(
    const rtsp::Request& /*request*/,
    const rtsp::Response& response)
{
    sendResponse(response);
}

bool BackSession::handleResponse(
    const rtsp::Request& request,
    const rtsp::Response& response) noexcept
{
    return _p->forwardContext->forwardToFrontSession(this, request, response);
}