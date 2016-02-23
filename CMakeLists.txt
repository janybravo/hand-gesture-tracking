cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)
project( BlurImage )
find_package( OpenCV )
include_directories( /usr/local/opt/opencv3/include )
add_executable( BlurImage HandDetector.cpp)
target_link_libraries( BlurImage ${OpenCV_LIBS} )
set(needed_features
        cxx_strong_enums
        cxx_constexpr
        cxx_auto_type)
target_compile_features(BlurImage PRIVATE ${needed_features})