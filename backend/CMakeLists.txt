# backend/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(HomeAssistantSmartHome VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 强制使用 MacPorts 路径
set(CMAKE_PREFIX_PATH "/opt/local" ${CMAKE_PREFIX_PATH})
set(Boost_ROOT "/opt/local")
set(OPENSSL_ROOT_DIR "/opt/local")

# 如果是macOS，设置一些特定选项
if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
endif()

# 设置包含目录和库路径
set(CPPREST_INCLUDE_DIR "/opt/local/include")
set(CPPREST_LIBRARY "/opt/local/lib/libcpprest.dylib")
set(BOOST_INCLUDE_DIR "/opt/local/include")
set(BOOST_SYSTEM_LIBRARY "/opt/local/lib/libboost_system-mt.dylib")
set(BOOST_THREAD_LIBRARY "/opt/local/lib/libboost_thread-mt.dylib")
set(OPENSSL_INCLUDE_DIR "/opt/local/include")
set(OPENSSL_SSL_LIBRARY "/opt/local/lib/libssl.dylib")
set(OPENSSL_CRYPTO_LIBRARY "/opt/local/lib/libcrypto.dylib")
set(SQLITE3_INCLUDE_DIR "/opt/local/include")
set(SQLITE3_LIBRARY "/opt/local/lib/libsqlite3.dylib")
set(LIBSERIALPORT_INCLUDE_DIR "/opt/local/include")
set(LIBSERIALPORT_LIBRARY "/opt/local/lib/libserialport.dylib")

# 收集所有源文件
file(GLOB_RECURSE SOURCE_FILES 
    src/*.cpp
)

# 收集所有头文件
file(GLOB_RECURSE HEADER_FILES
    include/*.h
    src/*.h
)

# 添加可执行文件
add_executable(smart_home_server
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

# 包含目录
target_include_directories(smart_home_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CPPREST_INCLUDE_DIR}
    ${BOOST_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${SQLITE3_INCLUDE_DIR}
    ${LIBSERIALPORT_INCLUDE_DIR}
)

# 链接库
target_link_libraries(smart_home_server PRIVATE
    ${CPPREST_LIBRARY}
    ${BOOST_SYSTEM_LIBRARY}
    ${BOOST_THREAD_LIBRARY}
    ${OPENSSL_SSL_LIBRARY}
    ${OPENSSL_CRYPTO_LIBRARY}
    ${SQLITE3_LIBRARY}
    ${LIBSERIALPORT_LIBRARY}
    pthread
)

# 设置编译选项
target_compile_options(smart_home_server PRIVATE
    -Wall
    -Wextra
    -O2
)

# 确保链接器能找到库
target_link_directories(smart_home_server PRIVATE
    /opt/local/lib
)