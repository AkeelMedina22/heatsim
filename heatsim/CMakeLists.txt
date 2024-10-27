# CMakeLists.txt : CMake project for heatsim, includes source and defines project-specific logic.

# Define the executable target
add_executable(heatsim "heatsim.cpp")

# Set the C++ standard to 20
target_compile_features(heatsim PUBLIC cxx_std_20)

# Enable OpenMP if available
find_package(OpenMP REQUIRED)
if (OpenMP_CXX_FOUND)
    target_link_libraries(heatsim PUBLIC OpenMP::OpenMP_CXX)
endif()
