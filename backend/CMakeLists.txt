cmake_minimum_required(VERSION 3.10)
project(HomeAssistantSmartHome)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖包
find_package(cpprestsdk REQUIRED)
find_package(PahoMqttCpp REQUIRED)
find_package(SQLite3 REQUIRED)

# 设置包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${cpprestsdk_INCLUDE_DIRS}
    ${PAHO_MQTT_CPP_INCLUDE_DIR}
    ${SQLITE3_INCLUDE_DIRS}
)

# 添加可执行文件
add_executable(smart_home_server
    src/main.cpp
)

# 链接库
target_link_libraries(smart_home_server
    ${cpprestsdk_LIBRARIES}
    ${PAHO_MQTT_CPP_LIBRARIES}
    ${SQLITE3_LIBRARIES}
    pthread
    ssl
    crypto
    z
)

# 安装目标（可选）
install(TARGETS smart_home_server DESTINATION bin)
