cmake_minimum_required (VERSION 3.19)
project("${CMAKE_PROJECT_NAME}")
include("$ENV{TOOLCHAIN_PATH}/CMakeLists.txt.in")

add_lib("lib${CMAKE_PROJECT_NAME}" "source/lib")
add_unittest(lib${CMAKE_PROJECT_NAME}
    ${LIB_JOB}
    ${LIB_CMD}
    ${LIB_ASP}
    ${LIB_NET}
    ${LIB_SHM}
    ${LIB_CFG}
    ${LIB_LOG}
    ${LIB_COM}
)
