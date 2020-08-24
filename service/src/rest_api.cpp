#include <libusockets.h>

#include <chrono>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <rest_api.hpp>
#include <sstream>

using namespace std::chrono;

namespace browser_service::rest {
    using namespace std::chrono;

    Statistics BrowserServiceAPI::_request_stats{};

    auto Statistics::to_string() const -> std::string {
        uint64_t       get_calls = _get_calls->load(std::memory_order::acquire);
        nlohmann::json json         = {{"request_counts", {{"get_calls", get_calls}}}};

        return json.dump();
    }

    BrowserServiceAPI::BrowserServiceAPI() {
        run();
    }

    BrowserServiceAPI::~BrowserServiceAPI() {
        stop();
    }

    auto BrowserServiceAPI::stop() -> void {
        spdlog::info("************************************************************************************");
        spdlog::info("*                     Browser Service REST API Stopping                           *");
        spdlog::info("************************************************************************************");

        decltype(_listen_sockets) listen_sockets(_config.rest_threads_count_);

        {
            std::lock_guard lock(_listen_sockets_mutex);
            listen_sockets = _listen_sockets;
        }

        for (auto& socket: listen_sockets) {
            if (nullptr != socket) {
                us_listen_socket_close(/* ssl */ false, socket);
            }
        }

        for (auto& thread : _api_threads) {
            thread->join();
        }
    }

    template<bool SSL>
    auto SendErrorResponse(uWS::HttpResponse<SSL>* response, const std::string& response_type, const std::string& status_code, const std::string& error_message) -> void {
        auto response_str = nlohmann::json {
                {"timestamp", duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count()},
                {"response_type", response_type},
                {"data", nullptr},
                {"error", error_message}}.dump();
        response->writeHeader("Access-Control-Allow-Origin", "*");
        response->cork([response, status_code, response_str = std::move(response_str)]() {
            response->writeStatus(status_code)->end(response_str);
        });
    }

    template<bool SSL>
    auto stream_data(uWS::HttpResponse<SSL>* response, const std::string& data, int offset = 0) -> void {
        if (!data.length()) {
            response->end();
            return;
        }
        response->writeStatus(OK_200);
        response->writeHeader("Access-Control-Allow-Origin", "*");
        if (!response->tryEnd({data.data() + offset, data.length() - offset}, data.length()).first) {
            response->onWritable([response, data = std::move(data)](int offset) {
                        stream_data<SSL>(response, data, offset);
                        return false;
                    })
                    ->onAborted([data]() { spdlog::error("Data={} transfer aborted ", data); });
        }
    }

    auto BrowserServiceAPI::run() -> void {
        spdlog::info("************************************************************************************");
        spdlog::info("*                     Browser Service REST API Starting                           *");
        spdlog::info("************************************************************************************");

        std::transform(_api_threads.begin(), _api_threads.end(), _api_threads.begin(), [&](std::thread*) {
            return new std::thread([&]() {
                uWS::App()
                        .get("/start",
                             [&](auto* response, auto* request) {
                                try {
                                    _request_stats._get_calls->fetch_add(1, std::memory_order::acq_rel);
                                    auto query = request->getQuery();
                                    std::vector <std::string> res;
                                    bool result{false};
                                    boost::split(res, query, boost::is_any_of("&"));
                                    std::string browser, url;
                                    for (auto& str : res) {
                                        std::vector <std::string> param;
                                        boost::split(param, str, boost::is_any_of("="));
                                        if (param[0] == "browser") {
                                            spdlog::debug("start: method GET: browser = {}", param[1]);
                                            browser = param[1];
                                        } else if (param[0] == "url") {
                                            spdlog::debug("start: method GET: url = {}", param[1]);
                                            url = param[1];
                                        }
                                    }
                                    if(browser == "chrome") {
                                        std::string command{"google-chrome --new-window "};
                                        command += url;
                                        system(command.c_str());
                                        chrome_utl = url;
                                        result = true;
                                    } else if (browser == "firefox") {
                                        std::string command{"firefox --new-window "};
                                        command += url;
                                        system(command.c_str());
                                        _firefox_url = url;
                                        result = true;
                                    }
                                    std::string response_str;
                                    if (!result) {
                                        response_str =
                                                nlohmann::json{{"timestamp",
                                                                       duration_cast<milliseconds>(
                                                                               high_resolution_clock::now().time_since_epoch())
                                                                               .count()},
                                                               {"response_type", "start"},
                                                               {"data", "Invalid input"}}
                                                        .dump();
                                    } else {
                                        response_str =
                                                nlohmann::json{{"timestamp",
                                                                       duration_cast<milliseconds>(
                                                                               high_resolution_clock::now().time_since_epoch())
                                                                               .count()},
                                                               {"response_type", "start"},
                                                               {"data", "Success"}}
                                                        .dump();
                                    }
                                    response->cork([response, response_str = std::move(response_str)]() {
                                        stream_data(response, response_str);
                                    });
                                    
                                } catch (const std::exception &ex) {
                                    spdlog::error("/start request={}, error={}", request->getQuery(), ex.what());
                                    SendErrorResponse(response, REQUEST_TYPE_GET_HOLDINGS , SYSTEM_ERROR_500, ERROR_SYSTEM_ERROR_500);
                                    return;
                                }
                             })
                        .get("/stop",
                             [&](auto* response, auto* request) {
                                 try {
                                     _request_stats._get_calls->fetch_add(1, std::memory_order::acq_rel);
                                     auto query = request->getQuery();
                                     std::vector <std::string> res;
                                     boost::split(res, query, boost::is_any_of("&"));
                                     std::string browser;
                                     for (auto& str : res) {
                                         std::vector <std::string> param;
                                         boost::split(param, str, boost::is_any_of("="));
                                         if (param[0] == "browser") {
                                             spdlog::debug("start: method GET: browser = {}", param[1]);
                                             browser = param[1];
                                         }
                                     }
                                     if(browser == "chrome" || browser == "firefox") {
                                         // TODO
                                         result = true;
                                     }
                                     std::string response_str;
                                     if (!result) {
                                         response_str =
                                                 nlohmann::json{{"timestamp",
                                                                        duration_cast<milliseconds>(
                                                                                high_resolution_clock::now().time_since_epoch())
                                                                                .count()},
                                                                {"response_type", "stop"},
                                                                {"data", "Invalid input"}}
                                                         .dump();
                                     } else {
                                         response_str =
                                                 nlohmann::json{{"timestamp",
                                                                        duration_cast<milliseconds>(
                                                                                high_resolution_clock::now().time_since_epoch())
                                                                                .count()},
                                                                {"response_type", "stop"},
                                                                {"data", "Success"}}
                                                         .dump();
                                     }
                                     response->cork([response, response_str = std::move(response_str)]() {
                                         stream_data(response, response_str);
                                     });

                                 } catch (const std::exception &ex) {
                                     spdlog::error("/start request={}, error={}", request->getQuery(), ex.what());
                                     SendErrorResponse(response, REQUEST_TYPE_GET_HOLDINGS , SYSTEM_ERROR_500, ERROR_SYSTEM_ERROR_500);
                                     return;
                                 }
                             })
                        .get("/cleanup",
                             [&](auto* response, auto* request) {
                                 try {
                                     _request_stats._get_calls->fetch_add(1, std::memory_order::acq_rel);
                                     auto query = request->getQuery();
                                     std::vector <std::string> res;
                                     boost::split(res, query, boost::is_any_of("&"));
                                     std::string browser;
                                     for (auto& str : res) {
                                         std::vector <std::string> param;
                                         boost::split(param, str, boost::is_any_of("="));
                                         if (param[0] == "browser") {
                                             spdlog::debug("start: method GET: browser = {}", param[1]);
                                             browser = param[1];
                                         }
                                     }
                                     if(browser == "chrome" || browser == "firefox") {
                                         // TODO
                                         result = true;
                                     }
                                    std::string response_str;
                                     if (!result) {
                                         response_str =
                                                 nlohmann::json{{"timestamp",
                                                                        duration_cast<milliseconds>(
                                                                                high_resolution_clock::now().time_since_epoch())
                                                                                .count()},
                                                                {"response_type", "cleanup"},
                                                                {"data", "Invalid input"}}
                                                         .dump();
                                     } else {
                                         response_str =
                                                 nlohmann::json{{"timestamp",
                                                                        duration_cast<milliseconds>(
                                                                                high_resolution_clock::now().time_since_epoch())
                                                                                .count()},
                                                                {"response_type", "cleanup"},
                                                                {"data", "Success"}}
                                                         .dump();
                                     }
                                     response->cork([response, response_str = std::move(response_str)]() {
                                         stream_data(response, response_str);
                                     });

                                 } catch (const std::exception &ex) {
                                     spdlog::error("/start request={}, error={}", request->getQuery(), ex.what());
                                     SendErrorResponse(response, REQUEST_TYPE_GET_HOLDINGS , SYSTEM_ERROR_500, ERROR_SYSTEM_ERROR_500);
                                     return;
                                 }
                             })
                        .get("/geturl",
                             [&](auto* response, auto* request) {
                                 try {
                                     _request_stats._get_calls->fetch_add(1, std::memory_order::acq_rel);
                                     auto query = request->getQuery();
                                     std::vector <std::string> res;
                                     boost::split(res, query, boost::is_any_of("&"));
                                     std::string browser, url;
                                     for (auto& str : res) {
                                         std::vector <std::string> param;
                                         boost::split(param, str, boost::is_any_of("="));
                                         if (param[0] == "browser") {
                                             spdlog::debug("start: method GET: browser = {}", param[1]);
                                             browser = param[1];
                                         }
                                     }
                                     if(browser == "chrome") {
                                         url = _chrome_url;
                                         result = true;
                                     } else if(browser == "firefox") {
                                         url = _firefox_url;
                                         result = true;
                                     }
                                     std::string response_str;
                                     if (!result) {
                                         response_str =
                                                 nlohmann::json{{"timestamp",
                                                                        duration_cast<milliseconds>(
                                                                                high_resolution_clock::now().time_since_epoch())
                                                                                .count()},
                                                                {"response_type", "geturl"},
                                                                {"data", "Invalid input"}}
                                                         .dump();
                                     } else {
                                         response_str =
                                                 nlohmann::json{{"timestamp",
                                                                        duration_cast<milliseconds>(
                                                                                high_resolution_clock::now().time_since_epoch())
                                                                                .count()},
                                                                {"response_type", "geturl"},
                                                                {"data", url}}
                                                         .dump();
                                     }
                                     response->cork([response, response_str = std::move(response_str)]() {
                                         stream_data(response, response_str);
                                     });

                                 } catch (const std::exception &ex) {
                                     spdlog::error("/start request={}, error={}", request->getQuery(), ex.what());
                                     SendErrorResponse(response, REQUEST_TYPE_GET_HOLDINGS , SYSTEM_ERROR_500, ERROR_SYSTEM_ERROR_500);
                                     return;
                                 }
                             })
                        .get("health",
                             [&](auto* response, auto* request) {
                                 auto response_str = nlohmann::json{
                                         {"timestamp",
                                          duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch())
                                                  .count()}}.dump();

                                 response->cork([response, response_str = std::move(response_str)]() {
                                     stream_data(response, response_str);
                                 });
                             })
                        .get("/*",
                             [&](auto* response, auto* request) {
                                 spdlog::error("No such endpoint={}", request->getUrl());
                                 response->writeHeader("Access-Control-Allow-Origin", "*");
                                 response->cork([response]() {
                                     response->writeStatus("404 Not found!");
                                     response->end("Endpoint doesn't exists!");
                                 });
                             })
                        .listen(_config.rest_api_port_,
                                [&](auto* token) {
                                    std::ostringstream str;
                                    str << std::this_thread::get_id();
                                    if (token) {
                                        spdlog::info("Listening on port={}, thread={}", _config.rest_api_port_, str.str());
                                        {
                                            std::lock_guard lock(_listen_sockets_mutex);
                                            _listen_sockets.push_back(token);
                                        }
                                    } else {
                                        spdlog::info("Failed to listen on port={}, thread={}", _config.rest_api_port_,
                                                str.str());
                                    }
                                })
                        .run();
            });
        });
    }

}    // namespace browser_service::rest
