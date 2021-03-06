#include <thread>
#include <memory>

#include <CxxPtr/GlibPtr.h>

#include "Signalling/WsServer.h"
#include "Signalling/ServerSession.h"
#include "Client/WsClient.h"
#include "Client/ClientSession.h"
#include "GstStreaming/GstRtspReStreamer.h"
#include "GstStreaming/GstClient.h"

#include "../InverseProxyServer/InverseProxyServer.h"
#include "../InverseProxyClient/InverseProxyClient.h"

enum {
    RECONNECT_TIMEOUT = 5,
};


static std::unique_ptr<WebRTCPeer> CreateClientPeer()
{
    return std::make_unique<GstClient>();
}

static std::unique_ptr<rtsp::ClientSession> CreateClientSession (
    const std::string& sourceUri,
    const std::function<void (const rtsp::Request*) noexcept>& sendRequest,
    const std::function<void (const rtsp::Response*) noexcept>& sendResponse) noexcept
{
    return
        std::make_unique<ClientSession>(
            sourceUri,
            CreateClientPeer,
            sendRequest,
            sendResponse);
}

static void ClientDisconnected(client::WsClient* client) noexcept
{
    GSourcePtr timeoutSourcePtr(g_timeout_source_new_seconds(RECONNECT_TIMEOUT));
    GSource* timeoutSource = timeoutSourcePtr.get();
    g_source_set_callback(timeoutSource,
        [] (gpointer userData) -> gboolean {
            static_cast<client::WsClient*>(userData)->connect();
            return false;
        }, client, nullptr);
    g_source_attach(timeoutSource, g_main_context_get_thread_default());
}

int main(int argc, char *argv[])
{
    enum {
        FRONT_SERVER_PORT = 8080,
        BACK_SERVER_PORT = 8081,
    };

    const std::string sourceName = "source1";

    std::thread serverThread(
        [] () {
            InverseProxyServerConfig config {
                .frontPort = FRONT_SERVER_PORT,
                .backPort = BACK_SERVER_PORT,
            };
            InverseProxyServerMain(config);
        });

    std::thread streamSourceClientThread(
        [&sourceName] () {
            client::Config config {};
            config.server = "localhost";
            config.serverPort = BACK_SERVER_PORT;

            InverseProxyClientMain(config, sourceName);
        });

    std::thread clientThread(
        [&sourceName] () {
            client::Config config {};
            config.server = "localhost";
            config.serverPort = FRONT_SERVER_PORT;

            GMainContextPtr clientContextPtr(g_main_context_new());
            GMainContext* clientContext = clientContextPtr.get();
            g_main_context_push_thread_default(clientContext);
            GMainLoopPtr loopPtr(g_main_loop_new(clientContext, FALSE));
            GMainLoop* loop = loopPtr.get();

            client::WsClient client(
                config,
                loop,
                std::bind(
                    CreateClientSession,
                    sourceName,
                    std::placeholders::_1,
                    std::placeholders::_2),
                std::bind(ClientDisconnected, &client));

            if(client.init()) {
                client.connect();
                g_main_loop_run(loop);
            }
        });

    if(serverThread.joinable())
        serverThread.join();

    if(streamSourceClientThread.joinable())
        streamSourceClientThread.join();

    if(clientThread.joinable())
        clientThread.join();

    return 0;
}
