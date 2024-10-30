cmake_minimum_required(VERSION 3.10)
project(HomeWork)

# 查找 OpenCV
find_package(OpenCV REQUIRED)

# 收集源文件
file(GLOB_RECURSE SRC src/*.cpp)
list(APPEND SRC "main.cpp") # 添加 main.cpp 到源文件列表

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SRC})

# 链接 OpenCV 库
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# 添加包含目录
target_include_directories(${PROJECT_NAME} PUBLIC include)

# 设置可执行文件的输出路径
# set(EXECUTABLE_OUTPUT_PATH ../bin)