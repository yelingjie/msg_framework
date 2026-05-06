cmake_minimum_required(VERSION 3.10)
project(msg_framework C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 源码目录
set(FRAMEWORK_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(FRAMEWORK_PORT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/port)

# 输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${FRAMEWORK_SRC_DIR})

# ============================================================
# 选择 OS 适配层
# ============================================================
# 默认启用 RT-Thread，可通过 -DUSE_FREERTOS=ON 或 -DUSE_ZEPHYR=ON 切换
option(USE_FREERTOS "使用 FreeRTOS 适配层" OFF)
option(USE_ZEPHYR "使用 Zephyr 适配层" OFF)

if(USE_ZEPHYR)
    message(STATUS "使用 Zephyr 适配层")
    set(PORT_SRC ${FRAMEWORK_PORT_DIR}/zephyr/mf_port_zephyr.c)
elseif(USE_FREERTOS)
    message(STATUS "使用 FreeRTOS 适配层")
    set(PORT_SRC ${FRAMEWORK_PORT_DIR}/freertos/mf_port_freertos.c)
else()
    message(STATUS "使用 RT-Thread 适配层")
    set(PORT_SRC ${FRAMEWORK_PORT_DIR}/rtt/mf_port_rtt.c)
endif()

# ============================================================
# 编译静态库
# ============================================================
set(FRAMEWORK_SOURCES
    ${FRAMEWORK_SRC_DIR}/msg_framework.c
    ${PORT_SRC}
)

add_library(msg_framework STATIC ${FRAMEWORK_SOURCES})

# 目标属性
target_include_directories(msg_framework PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ============================================================
# 单元测试（可选）
# ============================================================
option(BUILD_TESTS "编译单元测试" OFF)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ============================================================
# 导出配置
# ============================================================
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION include/msg_framework)

install(TARGETS msg_framework
        ARCHIVE DESTINATION lib)
