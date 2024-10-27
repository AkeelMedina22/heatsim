cmake_minimum_required(VERSION 3.15)
project(heatsim VERSION 1.0)

# Set the C++ standard and specify clang-cl as the compiler
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER clang-cl)
set(CMAKE_C_COMPILER clang-cl)

# Enable OpenMP support
find_package(OpenMP REQUIRED)
if (OpenMP_CXX_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -openmp")
endif()

# Include directories for GLFW and GLAD
include_directories("C:/Users/akeel/OneDrive/Documents/libs/GLFW/include")
include_directories("C:/Users/akeel/OneDrive/Documents/libs/GLAD/include")

# Link GLFW library
set(GLFW_LIB_PATH "C:/Users/akeel/OneDrive/Documents/libs/GLFW/lib-vc2022")
link_directories(${GLFW_LIB_PATH})

# Add GLAD as a library
add_library(glad STATIC "C:/Users/akeel/OneDrive/Documents/libs/GLAD/src/glad.c")

# Define the executable and add the main source file
add_executable(${PROJECT_NAME} heatsim/heatsim.cpp)

# Link libraries (GLFW, GLAD, OpenMP)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw3 glad OpenMP::OpenMP_CXX)

# Additional clang-cl specific flags for OpenMP
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -openmp)
endif()
