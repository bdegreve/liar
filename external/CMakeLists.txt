include(ExternalProject)

macro(_set_if_empty varname value vartype)
    if (NOT ${varname})
        set(${varname} "${value}" CACHE "${vartype}" "" FORCE)
    endif()
endmacro()

option(JPEG_BUILD OFF)
if(JPEG_BUILD)
    set(JPEG_URL "http://www.ijg.org/files/jpegsrc.v9.tar.gz" CACHE STRING "URL to JPEG library source code")
    set(_configure)
    if (MSVC)
        set(_make nmake -fmakefile.vc nodebug=1 libjpeg.lib)
        list(APPEND _configure
            "${CMAKE_COMMAND}" -E copy_if_different jconfig.vc jconfig.h
        )
        if (MSVC_VERSION GREATER  1699) # >= 1700, MSVC11+
            # oh great, they dropped win32.mak 
            find_file(JPEG_WIN32_MAK "Win32.Mak"
                HINTS "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Include"
                )
            if (JPEG_WIN32_MAK)
                list(APPEND _configure
                    && "${CMAKE_COMMAND}" -E copy_if_different "${JPEG_WIN32_MAK}" Win32.Mak
                    )  
            else()
                message(WARNING "No Win32.Mak, building libjpeg may fail. set JPEG_WIN32_MAK")
            endif()
        endif()
    endif()
    ExternalProject_Add(
        external_jpeg
        PREFIX jpeg
        URL "${JPEG_URL}"
        CONFIGURE_COMMAND ${_configure}
        BUILD_COMMAND ${_make}
        BUILD_IN_SOURCE 1
        INSTALL_COMMAND ""
        )
    set(_jpegdir "${CMAKE_CURRENT_BINARY_DIR}/jpeg/src/external_jpeg")
    _set_if_empty(JPEG_INCLUDE_DIR "${_jpegdir}" PATH)
    _set_if_empty(JPEG_LIBRARY "${_jpegdir}/libjpeg.lib" FILEPATH)
    mark_as_advanced(JPEG_URL JPEG_INCLUDE_DIR JPEG_LIBRARY)
endif()

option(ZLIB_BUILD OFF)
if(ZLIB_BUILD)
    set(ZLIB_URL "http://zlib.net/zlib-1.2.8.tar.gz" CACHE STRING "URL to zlib source code")
    mark_as_advanced(ZLIB_URL)
    ExternalProject_Add(
        external_zlib
        PREFIX zlib
        URL "${ZLIB_URL}"
        CMAKE_CACHE_ARGS 
            "-DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo"
            "-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>" 
        BUILD_COMMAND "${CMAKE_COMMAND}" --build <BINARY_DIR> --config RelWithDebInfo
        INSTALL_COMMAND "${CMAKE_COMMAND}" --build <BINARY_DIR> --config RelWithDebInfo --target install
        )
    set(_zlibdir "${CMAKE_CURRENT_BINARY_DIR}/zlib")
    _set_if_empty(ZLIB_INCLUDE_DIR "${_zlibdir}/include" PATH)
    _set_if_empty(ZLIB_LIBRARY "${_zlibdir}/lib/zlib.lib" FILEPATH)
    _set_if_empty(ZLIB_REDIST "${_zlibdir}/bin/zlib.dll" FILEPATH)
    mark_as_advanced(ZLIB_URL ZLIB_INCLUDE_DIR ZLIB_LIBRARY ZLIB_REDIST)
endif()

option(OpenEXR_BUILD OFF)
if(OpenEXR_BUILD)
    set(OpenEXR_IlmBase_URL "https://github.com/bdegreve/openexr/branches/v2.2fixes/IlmBase" CACHE STRING "URL to IlmBase source code")
    set(OpenEXR_URL "https://github.com/bdegreve/openexr/branches/v2.2fixes/OpenEXR" CACHE STRING "URL to OpenEXR source code")
    mark_as_advanced(OpenEXR_IlmBase_URL OpenEXR_URL)
    find_package(ZLIB)
    if (ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY AND Lass_PYTHON_EXECUTABLE)  
        ExternalProject_Add(
            external_ilmbase
            PREFIX ilmbase
            SVN_REPOSITORY "${OpenEXR_IlmBase_URL}"
            CMAKE_CACHE_ARGS 
                "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
                "-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>"
                "-DBUILD_TESTING:BOOL=OFF"
            BUILD_COMMAND "${CMAKE_COMMAND}" --build <BINARY_DIR> --config RelWithDebInfo
            INSTALL_COMMAND "${CMAKE_COMMAND}" --build <BINARY_DIR> --config RelWithDebInfo --target install
            INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/openexr"
            )
        ExternalProject_Add(
            external_openexr
            PREFIX openexr
            SVN_REPOSITORY "${OpenEXR_URL}"
            CMAKE_CACHE_ARGS 
                "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
                "-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>"
                "-DILMBASE_PACKAGE_PREFIX:PATH=<INSTALL_DIR>"
                "-DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR}"
                "-DZLIB_LIBRARY:PATH=${ZLIB_LIBRARY}"
            BUILD_COMMAND "${CMAKE_COMMAND}" --build <BINARY_DIR> --config RelWithDebInfo
            INSTALL_COMMAND "${CMAKE_COMMAND}" --build <BINARY_DIR> --config RelWithDebInfo --target install
            INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/openexr"
            )
        add_dependencies(external_openexr external_ilmbase)
        if(ZLIB_BUILD)
            add_dependencies(external_openexr external_zlib)
        endif()
        set(_openexrdir "${CMAKE_CURRENT_BINARY_DIR}/openexr")
        _set_if_empty(OpenEXR_DIR "${_openexrdir}" PATH)
        _set_if_empty(OpenEXR_INCLUDE_DIR "${_openexrdir}/include" PATH)
        mark_as_advanced(OpenEXR_INCLUDE_DIR)
        foreach(_comp Half Iex IlmThread IlmImf)
            if (MSVC_IDE)
                _set_if_empty(OpenEXR_${_comp}_LIBRARY "${_openexrdir}/lib/${_comp}-2_2.lib" FILEPATH)
                _set_if_empty(OpenEXR_${_comp}_REDIST "${_openexrdir}/lib/${_comp}-2_2.dll" FILEPATH)
                mark_as_advanced(OpenEXR_${_comp}_LIBRARY OpenEXR_${_comp}_REDIST)
            endif()
        endforeach()
    endif()
endif()

option(LCMS2_BUILD OFF)
if(LCMS2_BUILD)
    set(LCMS2_URL "http://downloads.sourceforge.net/project/lcms/lcms/2.4/lcms2-2.4.zip" CACHE STRING "URL to LCMS2 library source code")
    ExternalProject_Add(
        external_lcms2
        PREFIX lcms2
        URL "${LCMS2_URL}"
        PATCH_COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/Lcms2-CMakeList.txt" "./CMakeLists.txt"
        CMAKE_CACHE_ARGS 
            "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
            "-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>"
        INSTALL_COMMAND "${CMAKE_COMMAND}" --build <BINARY_DIR> --config RelWithDebInfo --target install
        )
    set(_lcms2dir "${CMAKE_CURRENT_BINARY_DIR}/lcms2")
    _set_if_empty(LCMS2_INCLUDE_DIR "${_lcms2dir}/include" PATH)
    _set_if_empty(LCMS2_LIBRARY "${_lcms2dir}/lib/RelWithDebInfo/lcms2.lib" FILEPATH)
    _set_if_empty(LCMS2_REDIST "${_lcms2dir}/bin/RelWithDebInfo/lcms2.dll" FILEPATH)
    mark_as_advanced(LCMS2_URL LCMS2_INCLUDE_DIR LCMS2_LIBRARY LCMS2_REDIST)
endif()