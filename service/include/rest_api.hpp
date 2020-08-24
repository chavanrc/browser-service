#pragma once

#include <libusockets.h>
#include <uWebSockets/App.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>

namespace browser_service::rest {

    struct Statistics {
        std::atomic<uint64_t>* _get_calls;
        auto                   to_string() const -> std::string;
    };

    class BrowserServiceAPI {
    public:
        BrowserServiceAPI();
        ~BrowserServiceAPI();


        auto run() -> void;
        auto stop() -> void;

    private:
        std::vector<std::thread*>        _api_threads;
        std::vector<us_listen_socket_t*> _listen_sockets;
        std::mutex                       _listen_sockets_mutex;
        std::atomic<std::string>         _chrome_url;
        std::atomic<std::string>         _firefox_url;

        static Statistics _request_stats;
    };
}    // namespace browser_service::rest
