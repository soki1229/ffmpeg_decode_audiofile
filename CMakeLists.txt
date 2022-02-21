cmake_minimum_required(VERSION 3.0.0)
project(audiofile_decode VERSION 0.1.0)

include(CTest)
enable_testing()


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

SET(FFMPEG_API_PATH C:/dev/ffmpeg)

include_directories(
    hdr
    ${FFMPEG_API_PATH}/include
)
link_directories(
    ${FFMPEG_API_PATH}/lib
)

link_libraries(
    avcodec
    avutil
    #avformat
)

SET(SRC
    src/main.cpp
    src/AudioDecoder.cpp
)

SET(HDR
    hdr/AudioDecoder.h
)

add_executable(audiofile_decode ${SRC} ${HDR})
