/*
 * websocket server
 *
 * @build   make examples
 * @server  bin/websocket_server_test 9999
 * @client  bin/websocket_client_test ws://127.0.0.1:9999/
 * @js      html/websocket_client.html
 *
 */

#include "WebSocketServer.h"
#include "EventLoop.h"
#include "htime.h"
#include "hssl.h"

/*
 * #define TEST_WSS 1
 *
 * @build   ./configure --with-openssl && make clean && make
 *
 * @server  bin/websocket_server_test 9999
 *
 * @client  bin/websocket_client_test ws://127.0.0.1:9999/
 *          bin/websocket_client_test wss://127.0.0.1:10000/
 *
 */
#define TEST_WSS 0

using namespace hv;

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s port\n", argv[0]);
        return -10;
    }
    int port = atoi(argv[1]);

    TimerID timerID = INVALID_TIMER_ID;
    WebSocketServerCallbacks ws;
    ws.onopen = [&timerID](const WebSocketChannelPtr& channel, const std::string& url) {
        printf("onopen: GET %s\n", url.c_str());
        // send(time) every 1s
        timerID = setInterval(1000, [channel](TimerID id) {
            char str[DATETIME_FMT_BUFLEN] = {0};
            datetime_t dt = datetime_now();
            datetime_fmt(&dt, str);
            channel->send(str);
        });
    };
    ws.onmessage = [](const WebSocketChannelPtr& channel, const std::string& msg) {
        printf("onmessage: %s\n", msg.c_str());
    };
    ws.onclose = [&timerID](const WebSocketChannelPtr& channel) {
        printf("onclose\n");
        if (timerID != INVALID_TIMER_ID) {
            killTimer(timerID);
            timerID = INVALID_TIMER_ID;
        }
    };

    websocket_server_t server;
    server.port = port;
#if TEST_WSS
    server.https_port = port + 1;
    hssl_ctx_init_param_t param;
    memset(&param, 0, sizeof(param));
    param.crt_file = "cert/server.crt";
    param.key_file = "cert/server.key";
    if (hssl_ctx_init(&param) == NULL) {
        fprintf(stderr, "SSL certificate verify failed!\n");
        return -20;
    }
#endif
    server.ws = &ws;
    websocket_server_run(&server);
    return 0;
}
