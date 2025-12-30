#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

class SmartHomeServer {
    http_listener listener;
    
public:
    SmartHomeServer(const std::string& url) : listener(utility::conversions::to_string_t(url)) {
        // 设置 API 端点
        listener.support(methods::GET, [this](http_request req) {
            handle_get(req);
        });
        
        listener.support(methods::POST, [this](http_request req) {
            handle_post(req);
        });
    }
    
    void start() {
        listener.open().then([]() {
            std::cout << "🚀 Smart Home Server started at http://localhost:8080" << std::endl;
        }).wait();
    }
    
private:
    void handle_get(http_request req) {
        auto path = utility::conversions::to_utf8string(req.relative_uri().path());
        
        if (path == "/api/status") {
            json::value response;
            response["status"] = json::value::string("online");
            response["version"] = json::value::string("1.0.0");
            response["system"] = json::value::string("HomeAssistant Smart Home");
            response["temperature"] = json::value::number(22.5);
            response["humidity"] = json::value::number(45.0);
            
            http_response res(status_codes::OK);
            res.set_body(response);
            res.headers().add("Access-Control-Allow-Origin", "*");
            req.reply(res);
        } else {
            req.reply(status_codes::NotFound);
        }
    }
    
    void handle_post(http_request req) {
        auto path = utility::conversions::to_utf8string(req.relative_uri().path());
        
        if (path == "/api/command") {
            req.extract_json().then([=](json::value body) {
                std::string command = utility::conversions::to_utf8string(body["command"].as_string());
                
                json::value response;
                response["received"] = json::value::string(command);
                response["status"] = json::value::string("processed");
                
                http_response res(status_codes::OK);
                res.set_body(response);
                res.headers().add("Access-Control-Allow-Origin", "*");
                req.reply(res);
                
                std::cout << "Received command: " << command << std::endl;
            }).wait();
        } else {
            req.reply(status_codes::NotFound);
        }
    }
};

int main() {
    try {
        std::cout << "Starting Smart Home Server..." << std::endl;
        
        SmartHomeServer server("http://localhost:8080");
        server.start();
        
        // 保持程序运行
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
