cmake_minimum_required(VERSION 3.0.0)
project(audiofile_decode VERSION 0.1.0)

include(CTest)
enable_testing()


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)


SET(FFMPEG_API_PATH C:/dev/ffmpeg)


SET(AUDIO_BASE_HDR AudioHandler/BaseModule/inc)
SET(AUDIO_BASE_SRC AudioHandler/BaseModule/src)

SET(AUDIO_DECODER_HDR AudioHandler/Decoder/inc)
SET(AUDIO_DECODER_SRC AudioHandler/Decoder/src)

SET(AUDIO_RESMPLR_HDR AudioHandler/Resampler/inc)
SET(AUDIO_RESMPLR_SRC AudioHandler/Resampler/src)

#INCLUDE EXTERNAL APIs FOR BUILD
include_directories(
    ${FFMPEG_API_PATH}/include
)
link_directories(
    ${FFMPEG_API_PATH}/lib
)
link_libraries(
    avcodec
    avformat
    avutil
    swresample
)

#INCLUDE INTERNAL SOURCES FOR BUILD
include_directories(
    ${AUDIO_BASE_HDR}
    ${AUDIO_DECODER_HDR}
    ${AUDIO_RESMPLR_HDR}
)
SET(SRC
    ${AUDIO_BASE_SRC}/Audio_Main.cpp
    ${AUDIO_DECODER_SRC}/AudioDecoder.cpp
    ${AUDIO_RESMPLR_SRC}/AudioResampler.cpp
)
SET(HDR
    ${AUDIO_BASE_HDR}/Audio_Definition.h
    ${AUDIO_DECODER_HDR}/AudioDecoder.h
    ${AUDIO_RESMPLR_HDR}/AudioResampler.h
)

add_executable(audiofile_decode ${SRC} ${HDR})
