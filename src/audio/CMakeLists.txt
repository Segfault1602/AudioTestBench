include(FetchContent)

FetchContent_Declare(
    rtaudio
    GIT_REPOSITORY https://github.com/thestk/rtaudio.git
    GIT_TAG 6.0.1
    )

set(RTAUDIO_API_ASIO ON CACHE BOOL "" FORCE)
FetchContent_GetProperties(rtaudio)
if (NOT rtaudio_POPULATED)
    FetchContent_Populate(rtaudio)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(RTAUDIO_BUILD_TESTING OFF CACHE BOOL "" FORCE)


    add_subdirectory(${rtaudio_SOURCE_DIR} ${rtaudio_BINARY_DIR})
    target_compile_definitions(rtaudio PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

FetchContent_Declare(
    libdsp
    GIT_REPOSITORY https://github.com/Segfault1602/libdsp.git
    GIT_TAG main
)

FetchContent_GetProperties(libdsp)
if (NOT libdsp_POPULATED)
    FetchContent_Populate(libdsp)
    set(LIBDSP_LIB_ONLY ON CACHE BOOL "" FORCE)
    set(LIBDSP_BUILD_TESTS OFF CACHE BOOL "" FORCE)

    add_subdirectory(${libdsp_SOURCE_DIR} ${libdsp_BINARY_DIR})
endif()

FetchContent_Declare(
    libsndfile
    GIT_REPOSITORY https://github.com/libsndfile/libsndfile.git
    GIT_TAG 1.2.2
    )

FetchContent_GetProperties(libsndfile)
if (NOT libsndfile_POPULATED)
    FetchContent_Populate(libsndfile)

    set(BUILD_PROGRAMS OFF CACHE BOOL "Don't build libsndfile programs!")
    set(BUILD_EXAMPLES OFF CACHE BOOL "Don't build libsndfile examples!")
    set(BUILD_REGTEST OFF CACHE BOOL "Don't build libsndfile regtest!")
    set(BUILD_PROGRAMS OFF CACHE BOOL "Don't build libsndfile programs!" FORCE)
    set(ENABLE_EXTERNAL_LIBS OFF CACHE BOOL "Disable external libs support!" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "Disable libsndfile tests!" FORCE)

    add_subdirectory(${libsndfile_SOURCE_DIR} ${libsndfile_BINARY_DIR})
    target_compile_definitions(sndfile PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

set(AUDIOLIB_SOURCE
    audio.cpp
    rtaudio_impl.cpp
    test_tone.cpp
    ring_buffer.cpp
    sndfile_manager_impl.cpp
    )

add_library(audiolib STATIC ${AUDIOLIB_SOURCE})
target_link_libraries(audiolib PRIVATE rtaudio dsp sndfile)
target_include_directories(audiolib PRIVATE ${libdsp_SOURCE_DIR}/include)


add_executable(test_buffer test.cpp ring_buffer.cpp)
target_link_libraries(test_buffer PRIVATE sndfile)
target_include_directories(test_buffer PRIVATE ${libsndfile_SOURCE_DIR}/include)

