cmake_minimum_required(VERSION 3.23)

project(libmikmod_examples C)

option(MIKMOD_BUILD_DOS "Enable DOS-specific deployment helpers" OFF)

set(LIBMIKMOD_EXAMPLES_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/BIN" CACHE PATH
    "Output directory for deployed example executables")

set(DOS32A_EXE "C:/WATCOM/binw/dos32a.exe" CACHE FILEPATH
    "Path to dos32a.exe from Open Watcom")

# Resolve library target
set(MIKMOD_EXAMPLE_LIB_TARGET "")
if(TARGET mikmod-static)
    set(MIKMOD_EXAMPLE_LIB_TARGET mikmod-static)
elseif(TARGET mikmod)
    set(MIKMOD_EXAMPLE_LIB_TARGET mikmod)
else()
    message(FATAL_ERROR "examples require libmikmod to be built in-tree.")
endif()

# Optional SDL example
find_package(SDL QUIET)

function(mikmod_example_common tgt)
    target_include_directories(${tgt} PRIVATE
        "${CMAKE_SOURCE_DIR}/libmikmod/include"
    )

    target_link_libraries(${tgt} PRIVATE ${MIKMOD_EXAMPLE_LIB_TARGET})

    if(MIKMOD_EXAMPLE_LIB_TARGET STREQUAL "mikmod-static")
        target_compile_definitions(${tgt} PRIVATE MIKMOD_STATIC)
    endif()

    set_target_properties(${tgt} PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
        FOLDER "examples"
    )

    if(CMAKE_C_COMPILER_ID STREQUAL "OpenWatcom")
        set_target_properties(${tgt} PROPERTIES SUFFIX ".exe")
    endif()
endfunction()

function(mikmod_example_deploy_dos tgt)
    add_custom_command(TARGET ${tgt} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LIBMIKMOD_EXAMPLES_BIN_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${tgt}>"
            "${LIBMIKMOD_EXAMPLES_BIN_DIR}/$<TARGET_FILE_NAME:${tgt}>"
        COMMENT "Deploying $<TARGET_FILE_NAME:${tgt}> to ${LIBMIKMOD_EXAMPLES_BIN_DIR}"
        VERBATIM
    )

    if(EXISTS "${DOS32A_EXE}")
        add_custom_command(TARGET ${tgt} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DOS32A_EXE}"
                "${LIBMIKMOD_EXAMPLES_BIN_DIR}/dos32a.exe"
            COMMENT "Deploying DOS32A as dos32a.exe to ${LIBMIKMOD_EXAMPLES_BIN_DIR}"
            VERBATIM
        )
    endif()
endfunction()

function(mikmod_add_example tgt output_name)
    add_executable(${tgt} ${ARGN})
    mikmod_example_common(${tgt})
    set_target_properties(${tgt} PROPERTIES OUTPUT_NAME "${output_name}")

    if(MIKMOD_BUILD_DOS AND (CMAKE_SYSTEM_NAME STREQUAL "DOS" OR CMAKE_C_COMPILER_ID STREQUAL "OpenWatcom"))
        mikmod_example_deploy_dos(${tgt})
    endif()
endfunction()

# playInteractive / play_int
mikmod_add_example(example_play_int play_int
    "${CMAKE_CURRENT_SOURCE_DIR}/playInteractive/play_int.c"
)

# simpleplay
mikmod_add_example(example_play_bas play_bas
    "${CMAKE_CURRENT_SOURCE_DIR}/simpleplay/splay.c"
)

# simpleplayFP
mikmod_add_example(example_play_fp play_fp
    "${CMAKE_CURRENT_SOURCE_DIR}/simpleplayFP/splayFP.c"
)

# simpleplayMEM
mikmod_add_example(example_play_mem play_mem
    "${CMAKE_CURRENT_SOURCE_DIR}/simpleplayMEM/splayMEM.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/simpleplayMEM/myloader.c"
)

# simpleplayMEM2
mikmod_add_example(example_play_me2 play_me2
    "${CMAKE_CURRENT_SOURCE_DIR}/simpleplayMEM2/splayMEM2.c"
)

# soundeffects
mikmod_add_example(example_sndfx sndfx
    "${CMAKE_CURRENT_SOURCE_DIR}/soundeffects/soundeffects.c"
)

# test
mikmod_add_example(example_test_nos test_nos
    "${CMAKE_CURRENT_SOURCE_DIR}/test/test.c"
)

# umxplay
mikmod_add_example(example_umxplay umxplay
    "${CMAKE_CURRENT_SOURCE_DIR}/umxplay/umxplay.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/umxplay/umxload.c"
)
target_include_directories(example_umxplay PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/umxplay"
)

# amiga example only when relevant
if(_MIKMOD_AMIGA)
    mikmod_add_example(example_amiga amiga
        "${CMAKE_CURRENT_SOURCE_DIR}/amiga.c"
    )
endif()

# SDL player if available
if(SDL_FOUND)
    add_executable(example_sdlplay
        "${CMAKE_CURRENT_SOURCE_DIR}/sdlplayer/sdlplay.c"
    )
    mikmod_example_common(example_sdlplay)
    target_include_directories(example_sdlplay PRIVATE ${SDL_INCLUDE_DIR})
    target_link_libraries(example_sdlplay PRIVATE ${SDL_LIBRARY})
    set_target_properties(example_sdlplay PROPERTIES OUTPUT_NAME "sdlplay")

    if(MIKMOD_BUILD_DOS AND (CMAKE_SYSTEM_NAME STREQUAL "DOS" OR CMAKE_C_COMPILER_ID STREQUAL "OpenWatcom"))
        mikmod_example_deploy_dos(example_sdlplay)
    endif()
else()
    message(STATUS "SDL not found: sdlplayer example will not be built")
endif()

message(STATUS "----------------------------------------")
message(STATUS "libmikmod examples configuration")
message(STATUS "  lib target      : ${MIKMOD_EXAMPLE_LIB_TARGET}")
message(STATUS "  SDL found       : ${SDL_FOUND}")
message(STATUS "  DOS deploy      : ${MIKMOD_BUILD_DOS}")
message(STATUS "----------------------------------------")