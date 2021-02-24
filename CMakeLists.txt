cmake_minimum_required (VERSION 3.19)
project("${CMAKE_PROJECT_NAME}")
include("$ENV{TOOLCHAIN_PATH}/CMakeLists.txt.in")

add_lib(libsvc "source/lib/svc")

SET(_deps
    ${LIB_JOB} ${LIB_CMD} ${LIB_ASP}
    ${LIB_NET} ${LIB_SHM}
    ${LIB_CFG} ${LIB_LOG}
    ${LIB_COM}
)

add_unittest(libsvc ${_deps})

aux_source_directory("source/lib/main" _main_files)
add_library(miu SHARED ${_main_files})
target_link_libraries(miu libsvc ${_deps})

add_bin(sample "source/bin/sample" miu)


