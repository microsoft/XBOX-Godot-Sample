include_guard(GLOBAL)

include(CMakeParseArguments)

function(gdk_detect_dependencies)
    set(one_value_args
        GDK_WINDOWS_OUT
        XSAPI_RUNTIME_DLL_OUT
        LIBHTTPCLIENT_RUNTIME_DLL_OUT
        PLAYFAB_CORE_RUNTIME_DLL_OUT
        PLAYFAB_GAMESAVE_RUNTIME_DLL_OUT
        PLAYFAB_SERVICES_RUNTIME_DLL_OUT
        PLAYFAB_MULTIPLAYER_RUNTIME_DLL_OUT
        PLAYFAB_PARTY_RUNTIME_DLL_OUT
    )
    cmake_parse_arguments(ARG "" "${one_value_args}" "" ${ARGN})

    if(DEFINED ENV{GameDKCoreLatest})
        file(TO_CMAKE_PATH "$ENV{GameDKCoreLatest}" _GDK_ROOT_FROM_ENV)
        if(EXISTS "${_GDK_ROOT_FROM_ENV}/windows/include/XGameRuntimeInit.h")
            set(_GDK_WINDOWS_DEFAULT "${_GDK_ROOT_FROM_ENV}/windows")
        endif()
    endif()

    if(NOT DEFINED _GDK_WINDOWS_DEFAULT AND DEFINED ENV{GameDKLatest})
        file(TO_CMAKE_PATH "$ENV{GameDKLatest}" _GDK_ROOT_FROM_LEGACY_ENV)
        if(EXISTS "${_GDK_ROOT_FROM_LEGACY_ENV}/windows/include/XGameRuntimeInit.h")
            set(_GDK_WINDOWS_DEFAULT "${_GDK_ROOT_FROM_LEGACY_ENV}/windows")
        endif()
    endif()

    if(NOT DEFINED _GDK_WINDOWS_DEFAULT)
        set(_GDK_ROOT "C:/Program Files (x86)/Microsoft GDK")
        file(GLOB _GDK_EDITIONS "${_GDK_ROOT}/[0-9]*")
        if(NOT _GDK_EDITIONS)
            message(FATAL_ERROR
                "Microsoft GDK not found.\n"
                "Install via: winget install Microsoft.Gaming.GDK\n"
                "Or set GDK_WINDOWS to your GDK windows layout path.")
        endif()
        list(SORT _GDK_EDITIONS ORDER DESCENDING)
        list(GET _GDK_EDITIONS 0 _GDK_EDITION_PATH)
        set(_GDK_WINDOWS_DEFAULT "${_GDK_EDITION_PATH}/windows")
    endif()

    set(GDK_WINDOWS "${_GDK_WINDOWS_DEFAULT}" CACHE PATH "Path to GDK windows layout directory")

    if(NOT EXISTS "${GDK_WINDOWS}/include/XGameRuntimeInit.h")
        message(FATAL_ERROR
            "GDK headers not found in the windows layout at: ${GDK_WINDOWS}\n"
            "Verify your GDK installation or set -DGDK_WINDOWS=<path>")
    endif()

    if(NOT EXISTS "${GDK_WINDOWS}/include/xsapi-c/services_c.h")
        message(FATAL_ERROR
            "Xbox Services API (XSAPI) headers not found in: ${GDK_WINDOWS}/include\n"
            "Ensure the GDK is installed with Xbox Extensions.")
    endif()

    if(NOT EXISTS "${GDK_WINDOWS}/include/httpClient/httpClient.h")
        message(FATAL_ERROR
            "libHttpClient headers not found in: ${GDK_WINDOWS}/include\n"
            "Ensure the GDK is installed with Xbox Extensions.")
    endif()

    if(NOT EXISTS "${GDK_WINDOWS}/include/playfab/multiplayer/PFMultiplayer.h")
        message(FATAL_ERROR
            "PlayFab Multiplayer headers not found in: ${GDK_WINDOWS}/include/playfab/multiplayer/\n"
            "Ensure the GDK is installed with Xbox Extensions.")
    endif()

    if(NOT EXISTS "${GDK_WINDOWS}/include/playfab/gamesave/PFGameSaveFiles.h")
        message(FATAL_ERROR
            "PlayFab Game Save headers not found in: ${GDK_WINDOWS}/include/playfab/gamesave/\n"
            "Ensure the GDK is installed with Xbox Extensions.")
    endif()

    if(NOT EXISTS "${GDK_WINDOWS}/include/playfab/party/Party.h")
        message(FATAL_ERROR
            "PlayFab Party headers not found in: ${GDK_WINDOWS}/include/playfab/party/\n"
            "Ensure the GDK is installed with Xbox Extensions.")
    endif()

    message(STATUS "GDK windows layout: ${GDK_WINDOWS}")

    if(ARG_GDK_WINDOWS_OUT)
        set(${ARG_GDK_WINDOWS_OUT} "${GDK_WINDOWS}" PARENT_SCOPE)
    endif()

    if(ARG_XSAPI_RUNTIME_DLL_OUT)
        # Match the linker import: addons/godot_gdk/CMakeLists.txt links against
        # Microsoft.Xbox.Services.C.Thunks[.Debug].lib whose DLL name is
        # Microsoft.Xbox.Services.C.Thunks[.Debug].dll. The GRDK ExtensionLibraries
        # variant ships a differently-named DLL (Microsoft.Xbox.Services.GDK.C.Thunks.dll)
        # which does NOT satisfy that import, so always deploy the SDK thunks DLL
        # alongside the addon binary so loading does not require GDK to be on PATH.
        set(${ARG_XSAPI_RUNTIME_DLL_OUT} "${GDK_WINDOWS}/bin/x64/Microsoft.Xbox.Services.C.Thunks$<$<CONFIG:Debug>:.Debug>.dll" PARENT_SCOPE)
    endif()

    if(ARG_LIBHTTPCLIENT_RUNTIME_DLL_OUT)
        # libHttpClient.dll is the SDK-shipped runtime that matches the linker import.
        set(${ARG_LIBHTTPCLIENT_RUNTIME_DLL_OUT} "${GDK_WINDOWS}/bin/x64/libHttpClient.dll" PARENT_SCOPE)
    endif()

    if(ARG_PLAYFAB_CORE_RUNTIME_DLL_OUT)
        set(${ARG_PLAYFAB_CORE_RUNTIME_DLL_OUT} "${GDK_WINDOWS}/bin/x64/PlayFabCore.dll" PARENT_SCOPE)
    endif()

    if(ARG_PLAYFAB_GAMESAVE_RUNTIME_DLL_OUT)
        set(${ARG_PLAYFAB_GAMESAVE_RUNTIME_DLL_OUT} "${GDK_WINDOWS}/bin/x64/PlayFabGameSave.dll" PARENT_SCOPE)
    endif()

    if(ARG_PLAYFAB_SERVICES_RUNTIME_DLL_OUT)
        set(${ARG_PLAYFAB_SERVICES_RUNTIME_DLL_OUT} "${GDK_WINDOWS}/bin/x64/PlayFabServices.dll" PARENT_SCOPE)
    endif()

    if(ARG_PLAYFAB_MULTIPLAYER_RUNTIME_DLL_OUT)
        set(${ARG_PLAYFAB_MULTIPLAYER_RUNTIME_DLL_OUT} "${GDK_WINDOWS}/bin/x64/PlayFabMultiplayer.dll" PARENT_SCOPE)
    endif()

    if(ARG_PLAYFAB_PARTY_RUNTIME_DLL_OUT)
        set(${ARG_PLAYFAB_PARTY_RUNTIME_DLL_OUT} "${GDK_WINDOWS}/bin/x64/Party.dll" PARENT_SCOPE)
    endif()
endfunction()
