project(MDNSREFLECTOR)

# cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
#set(CMAKE_CXX_FLAGS_RELEASE -Ofast)
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
cmake_policy(SET CMP0063 NEW)
cmake_policy(SET CMP0057 NEW)

#add_compile_options(-Wall -Wextra -Werror -fsanitize=address -std=c++2a -Og -DDEBUG)
#add_compile_options(-Wall -Wextra -Werror -std=c++2a -O3 -Wc++2a-extensions -DNDEBUG)
# add_compile_options(-Wall -Wextra -Werror -std=c++2a -O3 -Wc++2a-extensions)

#set(ASAN_OPTIONS detect_leaks=1)

# find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED)
#find_package(Threads REQUIRED)

#pkg_check_modules(FMT REQUIRED fmt)
# find_package(fmt REQUIRED)
# find_package(nlohmann_json REQUIRED)
#pkg_check_modules(NLOHMANN_JSON REQUIRED nlohmann_json)

# find_library(FASTFOURIER_FRAMEWORK "FastFourier" REQUIRED)

add_executable(mdnsreflector main.cpp)
#target_include_directories(testapp PRIVATE ${CMAKE_SOURCE_DIR} ${JUCE_INCLUDE_DIRS})
#target_compile_definitions(testapp PRIVATE ${JUCE_CFLAGS_OTHER} ${SPDLOG_CFLAGS})

# add_executable(spatializer ../embedded-software/dsp/blocks/spatialization/spatialization_test.cpp)
# target_link_libraries(spatializer -fsanitize=address "-framework Accelerate" fmt::fmt)

#include_directories(testapp "${FASTFOURIER_FRAMEWORK}/Headers/" "${nlohmann_json_INCLUDE_DIR}")

# include_directories(testapp "${FASTFOURIER_FRAMEWORK}/Headers/" "/usr/local/include/")
# target_link_libraries(testapp -fsanitize=address "-framework Accelerate" ${FASTFOURIER_FRAMEWORK} fmt::fmt)

# add_executable(fft_v3 fft_v3.cpp)
# target_link_libraries(fft_v3 -fsanitize=address "-framework Accelerate")

#add_executable(spatializer_test ../embedded-software/dsp/blocks/spatialization/spatialization_test.cpp)
#target_link_libraries(spatializer_test -fsanitize=address)

#install(TARGETS spatializer DESTINATION bin)
