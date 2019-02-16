
# #####################################
# Project Setup
# #####################################
cmake_minimum_required(VERSION 3.2)

project(light_io C)

set(CMAKE_C_STANDARD 11)



# #####################################
# Compiler Setup
# #####################################
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall)
    add_compile_options(-Werror)
    add_compile_options(-Wextra)
    add_compile_options(-pedantic)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_definitions(/GL) # global program optimization
    add_definitions(/Oi) # enable instrinsic functions
    add_definitions(/Ot) # favor fast code
    add_definitions(/Oy) # Omit frame pointers
    add_definitions(/permissive-) # enable conformance mode
    add_definitions(/Wall) # enable all warnings
    add_definitions(/sdl)
    add_definitions(/W3) # production-quality warnings
    add_definitions(/WX) # warnings are errors
    add_definitions(/Zc:rvalueCast /Zc:strictStrings /Zc:inline)

    # Prevent libraries from being rebuilt
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    /LTCG:INCREMENTAL")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LTCG:INCREMENTAL")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /LTCG:INCREMENTAL")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /LTCG:INCREMENTAL")

    add_definitions(/wd4146) # unsigned negation using the '-' operator
    add_definitions(/wd4996) # insecure use of wcstombs
    add_definitions(/wd4365) # Impplicit conversion from signed to unsigned (built into the standard library :/ )
    add_definitions(/wd4820) # Padding being added to structures using "alignas"... also built into the standard libary
    add_definitions(/wd4571) # Catch semantics changed
    add_definitions(/wd4625) # Implicly deleted copy constructor
    add_definitions(/wd4626) # Implicly deleted copy operator
    add_definitions(/wd5026) # Implicly deleted move constructor
    add_definitions(/wd5027) # Implicly deleted move operator
    add_definitions(/wd4774) # Format string is not a string literal in sprintf_s
    add_definitions(/wd4868) # compiler may not enforce left-to-right evaluation order in braced initializer list
    add_definitions(/wd5039) # An 'extern "C"' function contains something which might throw an exception
    add_definitions(/wd4244) # possible loss of data when converting between time_t and unsigned int
endif()


# #####################################
# Sources & Headers
# #####################################
include_directories(include)
set(SOURCE_DIR src)

set(SOURCE_FILES
    ${SOURCE_DIR}/lio_files.c
    ${SOURCE_DIR}/lio_utils.c)



if(NOT WIN32)
    list(APPEND SOURCE_FILES ${SOURCE_DIR}/lio_paths_nix.c)
else()
    list(APPEND SOURCE_FILES ${SOURCE_DIR}/lio_paths_win.c)
endif()



# #####################################
# Target Libraries
# #####################################
add_library(${PROJECT_NAME} ${SOURCE_FILES})

if (WIN32)
    set(BUILD_SHARED_LIBS ON)
    target_link_libraries(${PROJECT_NAME} Shlwapi Kernel32)
endif()



# #####################################
# Target Executables
# #####################################
add_executable(path_test test/path_test.c)
target_link_libraries(path_test ${PROJECT_NAME})



# #####################################
# Installation
# #####################################
install(
    TARGETS
        ${PROJECT_NAME}
    RUNTIME DESTINATION
        bin
    LIBRARY DESTINATION
        lib
    ARCHIVE DESTINATION
        lib)


install(
    DIRECTORY
        include/light_io
    DESTINATION
        include/light_io)



# #####################################
# Testing
# #####################################
enable_testing()
include(CTest)

if (BUILD_TESTING)
    add_test(path_test path_test)
endif()
