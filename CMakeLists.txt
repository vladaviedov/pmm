cmake_minimum_required(VERSION 3.5)

# Project setup
project(pmm VERSION 0.1)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -g)

# LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SRC src/*.c)
add_executable(pmm ${SRC})
target_link_libraries(pmm -lcrypto -lalpm)
