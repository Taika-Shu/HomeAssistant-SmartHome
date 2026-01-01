#define CPPREST_FORCE_HTTP_CLIENT
#define CPPREST_FORCE_HTTP_LISTENER

#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <mutex>
#include <fstream>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace utility::conversions;

// 全局配置常量
const std::string DISCORD_WEBHOOK_URL = "https://discord.com/api/webhooks/1453765565511368819/MofgFzUw2Kz5IU5L1xgg1vU3BF6XR1_zdTgLc3cY5x0pA9m1qnGznkz1c04NUuVx8gEe";
const std::string HOME_ASSISTANT_URL = "http://192.168.64.2:8123";
const std::string HOME_ASSISTANT_TOKEN = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiI1ZTAwNzlhMWI0MTc0MTZkYTRjMzdkYjA0YjAzNjJlYiIsImlhdCI6MTc2NjY3NzgwNiwiZXhwIjoyMDgyMDM3ODA2fQ.dPcpHWnbK2cvWbQ5xNKW-3B6i0XQydqycjWRUJFfcw0";

// 全局状态变量
std::mutex g_data_mutex;
double g_indoor_temp = -15.2;  // 默认模拟值
double g_indoor_humidity = 85.3; // 默认模拟值
double g_outdoor_temp = -20.1;
double g_outdoor_humidity = 90.5;
int g_security_mode = 1; // 1=家庭, 2=办公室, 3=银行, 4=博物馆
bool g_alarm_active = false;
std::chrono::steady_clock::time_point g_last_alert_time = std::chrono::steady_clock::now();
std::chrono::steady_clock::time_point g_last_ok_time = std::chrono::steady_clock::now();

// 读取 Pico 传感器数据（带降级）
std::string read_pico_sensor() {
    try {
        std::ifstream serial_port("/dev/tty.usbmodem14101");
        if (!serial_port) {
            std::cerr << "⚠️ Pico not connected. Using simulated data." << std::endl;
            // 返回模拟芬兰冬季数据
            return "T:" + std::to_string(g_indoor_temp) + ",H:" + std::to_string(g_indoor_humidity);
        }
        
        std::string line;
        if (std::getline(serial_port, line)) {
            return line; // 格式: "T:-15.2,H:85.3"
        }
        return "T:" + std::to_string(g_indoor_temp) + ",H:" + std::to_string(g_indoor_humidity);
    } catch (const std::exception& e) {
        std::cerr << "Serial error: " << e.what() << std::endl;
        return "T:" + std::to_string(g_indoor_temp) + ",H:" + std::to_string(g_indoor_humidity);
    }
}

// 解析 Pico 传感器数据
void parse_sensor_data(const std::string& data_str) {
    std::lock_guard<std::mutex> lock(g_data_mutex);
    try {
        // 假设数据格式为 "T:-15.2,H:85.3"
        size_t temp_pos = data_str.find("T:");
        size_t hum_pos = data_str.find(",H:");
        
        if (temp_pos != std::string::npos && hum_pos != std::string::npos) {
            std::string temp_str = data_str.substr(temp_pos + 2, hum_pos - (temp_pos + 2));
            std::string hum_str = data_str.substr(hum_pos + 3);
            
            g_indoor_temp = std::stod(temp_str);
            g_indoor_humidity = std::stod(hum_str);
        }
    } catch (...) {
        std::cerr << "Error parsing sensor data: " << data_str << std::endl;
    }
}

// 发送消息到 Discord
void send_to_discord(const std::string& message) {
    try {
        web::http::client::http_client client(to_string_t(DISCORD_WEBHOOK_URL));
        
        json::value payload;
        payload[U("content")] = json::value::string(to_string_t(message));
        payload[U("username")] = json::value::string(U("🏠 Finnish Home Guard"));
        
        auto request = http_request(methods::POST);
        request.set_body(payload);
        request.headers().add(U("Content-Type"), U("application/json"));
        
        auto response = client.request(request).get();
        
        std::cout << "Discord response status: " << response.status_code() << std::endl;
        
        if (response.status_code() != status_codes::OK && response.status_code() != 204) {
            auto error_body = response.extract_string().get();
            std::cerr << "Discord error: " << utility::conversions::to_utf8string(error_body) << std::endl;
        } else {
            std::cout << "✅ Message sent to Discord: " << message << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Discord send error: " << e.what() << std::endl;
    }
}

// 获取 Home Assistant 数据（简化模拟，实际应调用 API）
std::pair<double, double> get_home_assistant_data() {
    // 这里应实现真实的 HA API 调用
    // 模拟返回值
    return std::make_pair(-20.1, 90.5);
}

// 检查是否应该触发警报
bool should_alert() {
    auto now = std::chrono::steady_clock::now();
    auto time_since_last_ok = std::chrono::duration_cast<std::chrono::seconds>(now - g_last_ok_time).count();
    
    // 根据安全模式和上次确认时间决定警报频率
    int alert_interval_seconds = 300; // 默认5分钟
    if (g_security_mode == 1) alert_interval_seconds = 300; // 家庭
    else if (g_security_mode == 2) alert_interval_seconds = 120; // 办公室
    else if (g_security_mode == 3) alert_interval_seconds = 60; // 银行
    else if (g_security_mode == 4) alert_interval_seconds = 30; // 博物馆
    
    if (time_since_last_ok < alert_interval_seconds) {
        return false; // 还在静默期内
    }
    
    auto time_since_last_alert = std::chrono::duration_cast<std::chrono::seconds>(now - g_last_alert_time).count();
    return time_since_last_alert >= alert_interval_seconds;
}

// 智能警报逻辑
void check_and_trigger_alerts() {
    std::lock_guard<std::mutex> lock(g_data_mutex);
    
    bool should_send_alert = false;
    std::string alert_message = "";
    
    // 检查室内温度
    if (g_indoor_temp < -15.0) {
        if (should_alert()) {
            alert_message = "🌡️ 室内极寒警报! 当前: " + std::to_string(g_indoor_temp) + "°C, 湿度: " + std::to_string(g_indoor_humidity) + "%. 请检查供暖系统!";
            should_send_alert = true;
        }
    }
    
    // 检查室内湿度
    if (g_indoor_humidity > 85.0) {
        if (should_alert()) {
            alert_message = "💧 室内高湿警报! 当前: " + std::to_string(g_indoor_temp) + "°C, 湿度: " + std::to_string(g_indoor_humidity) + "%. 请检查通风除湿!";
            should_send_alert = true;
        }
    }
    
    // 检查室外温度 (来自 HA)
    if (g_outdoor_temp < -20.0) {
        if (should_alert()) {
            alert_message = "❄️ 室外极端低温警报! 室外: " + std::to_string(g_outdoor_temp) + "°C. 请检查管道防冻!";
            should_send_alert = true;
        }
    }
    
    if (should_send_alert) {
        send_to_discord(alert_message);
        g_last_alert_time = std::chrono::steady_clock::now();
        g_alarm_active = true;
    }
}

// 主要的智能家居服务器类
class SmartHomeServer {
    http_listener listener;
    
public:
    SmartHomeServer(const std::string& url) : listener(utility::conversions::to_string_t(url)) {
        // 设置 GET 请求处理器
        listener.support(methods::GET, [this](http_request req) {
            handle_get(req);
        });
        
        // 设置 POST 请求处理器
        listener.support(methods::POST, [this](http_request req) {
            handle_post(req);
        });
        
        // 设置 OPTIONS 请求处理器 (CORS 预检)
        listener.support(methods::OPTIONS, [this](http_request req) {
            handle_options(req);
        });
    }
    
    void start() {
        listener.open().then([]() {
            std::cout << "🚀 Smart Home Server started at http://localhost:8080" << std::endl;
        }).wait();
    }
    
    void stop() {
        listener.close().then([]() {
            std::cout << "Server stopped." << std::endl;
        }).wait();
    }

private:
    void handle_get(http_request req) {
        auto path = utility::conversions::to_utf8string(req.relative_uri().path());
        
        if (path == "/api/status") {
            json::value response;
            {
                std::lock_guard<std::mutex> lock(g_data_mutex);
                response[U("status")] = json::value::string(U("running"));
                response[U("indoor")] = json::value::object({
                    {U("temp"), json::value::number(g_indoor_temp)},
                    {U("humidity"), json::value::number(g_indoor_humidity)}
                });
                response[U("outdoor")] = json::value::object({
                    {U("temp"), json::value::number(g_outdoor_temp)},
                    {U("humidity"), json::value::number(g_outdoor_humidity)}
                });
                response[U("security_mode")] = json::value::number(g_security_mode);
                response[U("alarm_active")] = json::value::boolean(g_alarm_active);
            }
            
            http_response res(status_codes::OK);
            res.set_body(response);
            add_cors_headers(res);
            req.reply(res);
            
        } else if (path == "/api/health") {
            json::value response;
            response[U("status")] = json::value::string(U("healthy"));
            response[U("timestamp")] = json::value::string(to_string_t(std::to_string(std::time(nullptr))));
            
            http_response res(status_codes::OK);
            res.set_body(response);
            add_cors_headers(res);
            req.reply(res);
            
        } else {
            http_response res(status_codes::NotFound);
            add_cors_headers(res);
            req.reply(res);
        }
    }
    
    void handle_post(http_request req) {
        auto path = utility::conversions::to_utf8string(req.relative_uri().path());
        
        // 支持两个端点格式
        if (path == "/api/command" || path == "/api/discord/command") {
            req.extract_json().then([=](json::value body) {
                std::string command = utility::conversions::to_utf8string(body[U("command")].as_string());
                
                std::string response_message = process_command(command);
                
                json::value response;
                response[U("received")] = json::value::string(to_string_t(command));
                response[U("status")] = json::value::string(U("processed"));
                response[U("response")] = json::value::string(to_string_t(response_message));
                
                http_response res(status_codes::OK);
                res.set_body(response);
                add_cors_headers(res);
                req.reply(res);
                
                std::cout << "Received command: " << command << std::endl;
                
            }).wait();
        } else {
            http_response res(status_codes::NotFound);
            add_cors_headers(res);
            req.reply(res);
        }
    }
    
    void handle_options(http_request req) {
        http_response res(status_codes::OK);
        add_cors_headers(res);
        req.reply(res);
    }
    
    void add_cors_headers(http_response& res) {
        res.headers().add(U("Access-Control-Allow-Origin"), U("*"));
        res.headers().add(U("Access-Control-Allow-Methods"), U("GET, POST, OPTIONS"));
        res.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type, Authorization"));
    }
    
    std::string process_command(const std::string& cmd) {
        std::string response;
        
        if (cmd == "!0") {
            // 系统概览
            {
                std::lock_guard<std::mutex> lock(g_data_mutex);
                response = "🔧 系统状态概览:\n"
                          "🏠 室内: " + std::to_string(g_indoor_temp) + "°C, " + std::to_string(g_indoor_humidity) + "%\n"
                          "🌡️ 室外: " + std::to_string(g_outdoor_temp) + "°C, " + std::to_string(g_outdoor_humidity) + "%\n"
                          "🛡️ 安全模式: Mode " + std::to_string(g_security_mode) + "\n"
                          "🔔 报警状态: " + (g_alarm_active ? "ACTIVE" : "NORMAL");
            }
            send_to_discord(response);
            
        } else if (cmd == "!1") {
            // 室外数据
            response = "🌡️ 室外环境: " + std::to_string(g_outdoor_temp) + "°C, 湿度 " + std::to_string(g_outdoor_humidity) + "%";
            send_to_discord(response);
            
        } else if (cmd == "!2") {
            // 演示模式
            response = "🎭 演示模式激活 - 芬兰冬季场景模拟\n"
                      "当前设置: 极低温度, 高湿度\n"
                      "系统将模拟极端环境下的响应";
            send_to_discord(response);
            // 可以设置模拟的极端值
            {
                std::lock_guard<std::mutex> lock(g_data_mutex);
                g_indoor_temp = -18.0; 
                g_indoor_humidity = 88.0;
            }
            
        } else if (cmd == "!3") {
            // 室内数据
            {
                std::lock_guard<std::mutex> lock(g_data_mutex);
                response = "🏠 室内环境: " + std::to_string(g_indoor_temp) + "°C, 湿度 " + std::to_string(g_indoor_humidity) + "%";
            }
            send_to_discord(response);
            
        } else if (cmd == "ok") {
            // 确认警报
            response = "✅ 报警确认。系统将在一段时间内保持静默。";
            send_to_discord(response);
            g_last_ok_time = std::chrono::steady_clock::now();
            g_alarm_active = false;
            
        } else {
            response = "❓ 未知命令: " + cmd + "\n可用命令: !0, !1, !2, !3, ok";
            send_to_discord(response);
        }
        
        return response;
    }
};

int main() {
    try {
        std::cout << "Starting Smart Home Server..." << std::endl;
        
        SmartHomeServer server("http://localhost:8080");
        server.start();
        
        // 启动时发送欢迎消息
        send_to_discord("🏠 Finnish Home Guard Server 启动成功! 系统就绪。");
        
        // 启动传感器读取线程
        std::thread sensor_thread([]() {
            while (true) {
                std::string sensor_data = read_pico_sensor();
                parse_sensor_data(sensor_data);
                
                // 检查并触发警报
                check_and_trigger_alerts();
                
                std::this_thread::sleep_for(std::chrono::seconds(10)); // 每10秒读取一次
            }
        });
        
        // 启动 Home Assistant 数据同步线程
        std::thread ha_thread([]() {
            while (true) {
                auto [temp, humidity] = get_home_assistant_data();
                {
                    std::lock_guard<std::mutex> lock(g_data_mutex);
                    g_outdoor_temp = temp;
                    g_outdoor_humidity = humidity;
                }
                std::this_thread::sleep_for(std::chrono::seconds(30)); // 每30秒同步一次
            }
        });
        
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        
        // 退出时清理
        sensor_thread.join();
        ha_thread.join();
        send_to_discord("🏠 Finnish Home Guard Server 关闭。");
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
