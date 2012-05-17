include(ExternalProject)

option(ZLIB_BUILD OFF)
if(ZLIB_BUILD)
    set(ZLIB_URL "http://zlib.net/zlib-1.2.7.tar.gz" CACHE STRING "URL to zlib source code")
    ExternalProject_Add(
        external_zlib
        PREFIX zlib
        URL "${ZLIB_URL}"
        CMAKE_CACHE_ARGS "-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>"
        )
endif()

option(JPEG_BUILD OFF)
if(JPEG_BUILD)
    set(JPEG_URL "http://www.ijg.org/files/jpegsrc.v8d.tar.gz" CACHE STRING "URL to JPEG library source code")
    if (MSVC)
        set(_make nmake -fmakefile.vc nodebug=1 libjpeg.lib)
        set(_jconfig jconfig.vc)
    endif()
    ExternalProject_Add(
        external_jpeg
        PREFIX jpeg
        URL "${JPEG_URL}"
        CONFIGURE_COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${_jconfig}" jconfig.h
        BUILD_COMMAND ${_make}
        BUILD_IN_SOURCE 1
        )
endif()