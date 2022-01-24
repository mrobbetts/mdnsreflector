project(MDNSREFLECTOR)

cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
cmake_policy(SET CMP0063 NEW)
cmake_policy(SET CMP0057 NEW)

IF(APPLE)
  set(CMAKE_THREAD_LIBS_INIT "-lpthread")
  set(CMAKE_HAVE_THREADS_LIBRARY 1)
  set(CMAKE_USE_WIN32_THREADS_INIT 0)
  set(CMAKE_USE_PTHREADS_INIT 1)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
ENDIF()

find_package(Threads REQUIRED)

find_package(Boost REQUIRED)

add_executable(mdnsreflector main.cpp)
target_link_libraries(mdnsreflector ${Boost_LIBRARIES} Threads::Threads)

install(TARGETS mdnsreflector DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

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
