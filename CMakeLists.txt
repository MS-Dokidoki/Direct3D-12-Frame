cmake_minimum_required(VERSION 3.0)
project(D3DAppFrame)

# 环境变量
set(PROJECT_ROOT "C:/Users/Administrator.PC-20191006TRUC/source/repos/DirectX")

# 工程配置
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /Subsystem:Windows")
add_definitions(-DUNICODE -D_UNICODE)

# 头文件与源文件
include_directories(${FRAME_PATH} ${PROJECT_ROOT})

add_executable(D3DApp D3DApp.cpp D3DHelper.cpp GameTimer.cpp GeometryGenerator.cpp Main.cpp)