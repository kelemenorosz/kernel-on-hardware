cmake_minimum_required(VERSION 3.20)
project(KERNEL_ON_HARDWARE LANGUAGES C ASM-ATT)
set(CMAKE_C_FLAGS "-ffreestanding -g -O2")

add_subdirectory(${CMAKE_SOURCE_DIR}/source/stub)
add_subdirectory(${CMAKE_SOURCE_DIR}/source/kernel)