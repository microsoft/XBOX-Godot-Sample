cmake_minimum_required(VERSION 3.29)

if(NOT DEFINED DUMPBIN OR NOT EXISTS "${DUMPBIN}")
    message(FATAL_ERROR "Pass -DDUMPBIN=<path to dumpbin.exe>")
endif()
if(NOT DEFINED BINARY OR NOT EXISTS "${BINARY}")
    message(FATAL_ERROR "Pass -DBINARY=<path to the built addon DLL>")
endif()

execute_process(
    COMMAND "${DUMPBIN}" /nologo /imports "${BINARY}"
    RESULT_VARIABLE status
    OUTPUT_VARIABLE imports
    ERROR_VARIABLE error
)
if(NOT status STREQUAL "0")
    message(FATAL_ERROR "Import inspection failed for ${BINARY}: ${error}")
endif()
if(imports MATCHES "GetCurrentPackageFamilyName")
    message(FATAL_ERROR "${BINARY} imports console-incompatible GetCurrentPackageFamilyName")
endif()
message(STATUS "${BINARY}: no GetCurrentPackageFamilyName import")
