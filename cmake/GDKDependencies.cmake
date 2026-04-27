include_guard(GLOBAL)

include(CMakeParseArguments)

function(gdk_detect_dependencies)
    set(one_value_args
        GDK_GAMEKIT_OUT
        XSAPI_ROOT_OUT
        LIBHTTPCLIENT_ROOT_OUT
        XSAPI_RUNTIME_DLL_OUT
        LIBHTTPCLIENT_RUNTIME_DLL_OUT
    )
    cmake_parse_arguments(ARG "" "${one_value_args}" "" ${ARGN})

    if(DEFINED ENV{GRDKLatest})
        set(_GDK_GAMEKIT_DEFAULT "$ENV{GRDKLatest}/GameKit")
    else()
        set(_GDK_ROOT "C:/Program Files (x86)/Microsoft GDK")
        file(GLOB _GDK_EDITIONS "${_GDK_ROOT}/[0-9]*")
        if(NOT _GDK_EDITIONS)
            message(FATAL_ERROR
                "Microsoft GDK not found.\n"
                "Install via: winget install Microsoft.Gaming.GDK\n"
                "Or set GDK_GAMEKIT to your GameKit path.")
        endif()
        list(SORT _GDK_EDITIONS ORDER DESCENDING)
        list(GET _GDK_EDITIONS 0 _GDK_EDITION_PATH)
        set(_GDK_GAMEKIT_DEFAULT "${_GDK_EDITION_PATH}/GRDK/GameKit")
    endif()

    set(GDK_GAMEKIT "${_GDK_GAMEKIT_DEFAULT}" CACHE PATH "Path to GDK GameKit directory")

    if(NOT EXISTS "${GDK_GAMEKIT}/Include/XGameRuntimeInit.h")
        message(FATAL_ERROR
            "GDK GameKit headers not found at: ${GDK_GAMEKIT}\n"
            "Verify your GDK installation or set -DGDK_GAMEKIT=<path>")
    endif()

    set(_XSAPI_EXT "${GDK_GAMEKIT}/../ExtensionLibraries")
    get_filename_component(_XSAPI_EXT "${_XSAPI_EXT}" ABSOLUTE)

    set(XSAPI_ROOT "${_XSAPI_EXT}/Xbox.Services.API.C" CACHE PATH "Path to Xbox Services API C")
    set(LIBHTTPCLIENT_ROOT "${_XSAPI_EXT}/Xbox.LibHttpClient" CACHE PATH "Path to libHttpClient")

    if(NOT EXISTS "${XSAPI_ROOT}/Include/xsapi-c/services_c.h")
        message(FATAL_ERROR
            "Xbox Services API (XSAPI) not found at: ${XSAPI_ROOT}\n"
            "Ensure the GDK is installed with Xbox Extensions, or set -DXSAPI_ROOT=<path>")
    endif()

    if(NOT EXISTS "${LIBHTTPCLIENT_ROOT}/Include/httpClient/httpClient.h")
        message(FATAL_ERROR
            "libHttpClient not found at: ${LIBHTTPCLIENT_ROOT}\n"
            "Ensure the GDK is installed with Xbox Extensions.")
    endif()

    message(STATUS "GDK GameKit: ${GDK_GAMEKIT}")
    message(STATUS "XSAPI: ${XSAPI_ROOT}")
    message(STATUS "libHttpClient: ${LIBHTTPCLIENT_ROOT}")

    if(ARG_GDK_GAMEKIT_OUT)
        set(${ARG_GDK_GAMEKIT_OUT} "${GDK_GAMEKIT}" PARENT_SCOPE)
    endif()

    if(ARG_XSAPI_ROOT_OUT)
        set(${ARG_XSAPI_ROOT_OUT} "${XSAPI_ROOT}" PARENT_SCOPE)
    endif()

    if(ARG_LIBHTTPCLIENT_ROOT_OUT)
        set(${ARG_LIBHTTPCLIENT_ROOT_OUT} "${LIBHTTPCLIENT_ROOT}" PARENT_SCOPE)
    endif()

    if(ARG_XSAPI_RUNTIME_DLL_OUT)
        set(${ARG_XSAPI_RUNTIME_DLL_OUT} "${XSAPI_ROOT}/Lib/x64/$<IF:$<CONFIG:Debug>,Debug,Release>/Microsoft.Xbox.Services.GDK.C.Thunks.dll" PARENT_SCOPE)
    endif()

    if(ARG_LIBHTTPCLIENT_RUNTIME_DLL_OUT)
        set(${ARG_LIBHTTPCLIENT_RUNTIME_DLL_OUT} "${LIBHTTPCLIENT_ROOT}/Redist/x64/libHttpClient.GDK.dll" PARENT_SCOPE)
    endif()
endfunction()
