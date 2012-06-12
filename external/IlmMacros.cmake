macro(ILM_CREATE_DLL target)
    if(BUILD_SHARED_LIBS)
        string(TOUPPER "${target}" _TARGET)
        set_target_properties("${target}"
            PROPERTIES
            LINK_FLAGS "/MAP /OPT:NOREF /OPT:ICF /INCREMENTAL:NO"
            DEFINE_SYMBOL "${_TARGET}_EXPORTS"
            )
        set(_map "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${target}.map")
        if (MSVC_IDE)
            set(_libdirs "${CMAKE_CURRENT_BINARY_DIR}/${target}.dir/${CMAKE_CFG_INTDIR}")
        else()
            set(_libdirs "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${target}.dir/${target}/")
        endif()
        list(APPEND _libdirs "${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_CFG_INTDIR}")
        if (ZLIB_LIBRARY)
            get_filename_component(_zlib_dir "${ZLIB_LIBRARY}" PATH)
            list(APPEND _libdirs "${_zlib_dir}")
        endif()        
        set(_lib "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${target}.lib")
        if (CMAKE_CL_64)
            set(_machine "X64")
        else()
            set(_machine "X86")
        endif()
        add_custom_command(
            TARGET "${target}"
            POST_BUILD
            COMMAND createDLL "-n${_map}" "-l${_libdirs}" "-i${_lib}" "-M${_machine}"
            )
    endif()
endmacro()


macro(ILM_INSTALL_TARGET target)
    if (CMAKE_CONFIGURATION_TYPES)
        set(_configs ${CMAKE_CONFIGURATION_TYPES})
    else()
        set(_configs "${CMAKE_BUILD_TYPE}")
    endif()
    foreach(_config ${_configs})
        install(
            TARGETS "${target}" 
            EXPORT IlmBase${_config}
            RUNTIME DESTINATION "bin/${_config}" CONFIGURATIONS "${_config}"
            LIBRARY DESTINATION "lib/${_config}" CONFIGURATIONS "${_config}"
            ARCHIVE DESTINATION "lib/${_config}" CONFIGURATIONS "${_config}"            
            )
    endforeach()
endmacro()


macro(ILM_INSTALL_HEADERS headers)
    install(
        FILES ${headers} ${ARGN}
        DESTINATION include
        )
endmacro()


macro(_glob_sources_and_headers dirname srcs hdrs)
    include_directories("${dirname}")
    file(GLOB _srcs "${dirname}/*.cpp")
    file(GLOB _hdrs "${dirname}/*.h")
endmacro()


macro(ILM_ADD_LIBRARY target)
    _glob_sources_and_headers("${target}" _srcs _hdrs)
    add_library("${target}"
        ${_srcs}
        ${_hdrs}
        )
    ILM_CREATE_DLL("${target}")
    ILM_INSTALL_TARGET("${target}")
    ILM_INSTALL_HEADERS(${_hdrs})
endmacro()


macro(ILM_ADD_EXECUTABLE target)
    _glob_sources_and_headers("${target}" _srcs _hdrs)
    add_executable("${target}"
        ${_srcs}
        ${_hdrs}
        )
    ILM_INSTALL_TARGET("${target}")
endmacro()
 

macro(ILM_ADD_TEST libname)
    set(_target "${libname}Test")
    _glob_sources_and_headers("${_target}" _srcs _hdrs)
    add_executable("${_target}"
        EXCLUDE_FROM_ALL
        ${_srcs}
        ${_hdrs}
        )
    target_link_libraries("${_target}"
        ${libname}
        )
    add_custom_command(
        TARGET "${_target}"
        POST_BUILD
        COMMAND "${_target}"
        )
endmacro()
