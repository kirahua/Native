#include "TestSerialize.h"

#include <cassert>

#include "RtspParser/RtspSerialize.h"


void TestSerialize() noexcept
{
    rtsp::Response response;
    response.protocol = rtsp::Protocol::RTSP_1_0;
    response.headerFields.emplace("CSeq", "1");
    response.headerFields.emplace("Public", "DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE");
    response.statusCode = 200;
    response.reasonPhrase = "OK";

    const std::string responseMessage = rtsp::Serialize(response);

    assert(responseMessage ==
        "RTSP/1.0 200 OK\r\n"
        "CSeq: 1\r\n"
        "Public: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE\r\n");
}
