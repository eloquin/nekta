cmake_minimum_required(VERSION 3.16)
project(nekta_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "" FORCE)
endif()

add_library(nekta_engine STATIC
  core/RtCheck.cpp
  params/ParamRegistry.cpp
  clock/Transport.cpp
  voice/SampleBuffer.cpp
  voice/Envelope.cpp
  voice/SamplerVoice.cpp
  voice/VoicePool.cpp
  roll/RollEngine.cpp
  dsp/Svf.cpp
  io/WavFile.cpp
  harness/Script.cpp
  Engine.cpp
)
target_include_directories(nekta_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(nekta_engine PRIVATE -Wall -Wextra -Wpedantic -Werror)

# Real-time checks (allocation guard) are on for tests and the harness so that
# every offline render exercises the same guard the audio thread will have.
option(NEKTA_RT_CHECKS "Hook operator new and fail on audio-thread allocation" ON)
if(NEKTA_RT_CHECKS)
  target_compile_definitions(nekta_engine PUBLIC NEKTA_RT_CHECKS=1)
endif()

add_executable(nekta_render harness/main.cpp)
target_link_libraries(nekta_render PRIVATE nekta_engine)
target_compile_options(nekta_render PRIVATE -Wall -Wextra -Wpedantic -Werror)

enable_testing()
set(NEKTA_TESTS
  test_params
  test_clock
  test_voice
  test_roll
  test_svf
  test_render
  test_fuzz
)
foreach(t ${NEKTA_TESTS})
  add_executable(${t} test/${t}.cpp)
  target_link_libraries(${t} PRIVATE nekta_engine)
  target_compile_options(${t} PRIVATE -Wall -Wextra -Wpedantic -Werror)
  add_test(NAME ${t} COMMAND ${t})
endforeach()
