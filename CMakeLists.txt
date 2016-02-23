cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)
project( HandDetector )
find_package( OpenCV )
include_directories( /usr/local/opt/opencv3/include )
add_executable( HandDetector HandDetector.cpp)
target_link_libraries( HandDetector ${OpenCV_LIBS} )
set(needed_features
        cxx_strong_enums
        cxx_constexpr
        cxx_auto_type)
target_compile_features(HandDetector PRIVATE ${needed_features})