cmake_minimum_required(VERSION 3.23)

project(libmikmod_examples C)

# ------------------------------------------------------------
# Common settings
# ------------------------------------------------------------
set(LIBMIKMOD_EXAMPLES_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/BIN" CACHE PATH
    "Output directory for deployed example executables")

set(DOS32A_EXE "C:/WATCOM/binw/dos32a.exe" CACHE FILEPATH
    "Path to dos32a.exe from Open Watcom")

# Resolve the in-tree libmikmod target.
set(MIKMOD_EXAMPLE_LIB_TARGET "")
if(TARGET mikmod-static)
    set(MIKMOD_EXAMPLE_LIB_TARGET mikmod-static)
elseif(TARGET mikmod)
    set(MIKMOD_EXAMPLE_LIB_TARGET mikmod)
else()
    message(FATAL_ERROR "examples require libmikmod to be built in-tree (mikmod-static or mikmod target missing).")
endif()

# SDL player is optional.
find_package(SDL QUIET)

# ------------------------------------------------------------
# Helper: common setup for every example target
# ------------------------------------------------------------
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

# ------------------------------------------------------------
# Helper: deploy executable to BIN and copy DOS32A for Watcom/DOS
# ------------------------------------------------------------
function(mikmod_example_deploy_dos tgt)
    if(NOT TARGET ${tgt})
        message(FATAL_ERROR "mikmod_example_deploy_dos: target '${tgt}' does not exist")
    endif()

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
    else()
        message(WARNING "DOS32A_EXE not found: ${DOS32A_EXE}")
    endif()
endfunction()

# ------------------------------------------------------------
# Helper: add a plain example with one source file
# ------------------------------------------------------------
function(mikmod_add_example tgt src)
    add_executable(${tgt} ${src})
    mikmod_example_common(${tgt})

    if(CMAKE_SYSTEM_NAME STREQUAL "DOS" OR CMAKE_C_COMPILER_ID STREQUAL "OpenWatcom")
        mikmod_example_deploy_dos(${tgt})
    endif()
endfunction()

# ------------------------------------------------------------
# playInteractive / play_int
# ------------------------------------------------------------
mikmod_add_example(play_int
    "${CMAKE_CURRENT_SOURCE_DIR}/playInteractive/play_int.c"
)
set_target_properties(play_int PROPERTIES OUTPUT_NAME "play_int")

if(BUILD_TESTING)
    add_test(NAME play_int-runs COMMAND play_int)
endif()

# ------------------------------------------------------------
# simpleplay / splay
# ------------------------------------------------------------
mikmod_add_example(play_bas
    "${CMAKE_CURRENT_SOURCE_DIR}/simpleplay/splay.c"
)
set_target_properties(play_bas PROPERTIES OUTPUT_NAME "play_bas")

# ------------------------------------------------------------
# simpleplayFP / splayFP
# ------------------------------------------------------------
mikmod_add_example(play_fp
    "${CMAKE_CURRENT_SOURCE_DIR}/simpleplayFP/splayFP.c"
)
set_target_properties(play_fp PROPERTIES OUTPUT_NAME "play_fp")

# ------------------------------------------------------------
# simpleplayMEM / splayMEM
# ------------------------------------------------------------
add_executable(play_mem
    "${CMAKE_CURRENT_SOURCE_DIR}/simpleplayMEM/splayMEM.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/simpleplayMEM/myloader.c"
)
mikmod_example_common(play_mem)
set_target_properties(play_mem PROPERTIES OUTPUT_NAME "play_mem")

if(CMAKE_SYSTEM_NAME STREQUAL "DOS" OR CMAKE_C_COMPILER_ID STREQUAL "OpenWatcom")
    mikmod_example_deploy_dos(play_mem)
endif()

# ------------------------------------------------------------
# simpleplayMEM2 / splayMEM2
# ------------------------------------------------------------
mikmod_add_example(play_me2
    "${CMAKE_CURRENT_SOURCE_DIR}/simpleplayMEM2/splayMEM2.c"
)
set_target_properties(play_me2 PROPERTIES OUTPUT_NAME "play_me2")

# ------------------------------------------------------------
# soundeffects
# ------------------------------------------------------------
mikmod_add_example(sndfx
    "${CMAKE_CURRENT_SOURCE_DIR}/soundeffects/soundeffects.c"
)
set_target_properties(sndfx PROPERTIES OUTPUT_NAME "sndfx")

# ------------------------------------------------------------
# umxplay
# ------------------------------------------------------------
add_executable(umxplay
    "${CMAKE_CURRENT_SOURCE_DIR}/umxplay/umxplay.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/umxplay/umxload.c"
)
mikmod_example_common(umxplay)
target_include_directories(umxplay PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/umxplay"
)
set_target_properties(umxplay PROPERTIES OUTPUT_NAME "umxplay")

if(CMAKE_SYSTEM_NAME STREQUAL "DOS" OR CMAKE_C_COMPILER_ID STREQUAL "OpenWatcom")
    mikmod_example_deploy_dos(umxplay)
endif()

# ------------------------------------------------------------
# test / test.c
# Keep as a smoke-style sample executable
# ------------------------------------------------------------
mikmod_add_example(test_nos
    "${CMAKE_CURRENT_SOURCE_DIR}/test/test.c"
)
set_target_properties(test_nos PROPERTIES OUTPUT_NAME "test_nos")

if(BUILD_TESTING)
    add_test(NAME test_nos-runs COMMAND test_nos)
endif()

# ------------------------------------------------------------
# amiga example
# Build only on Amiga-like configuration if ever enabled
# ------------------------------------------------------------
if(_MIKMOD_AMIGA)
    mikmod_add_example(amiga
        "${CMAKE_CURRENT_SOURCE_DIR}/amiga.c"
    )
    set_target_properties(amiga PROPERTIES OUTPUT_NAME "amiga")
endif()

# ------------------------------------------------------------
# SDL player (optional)
# ------------------------------------------------------------
if(SDL_FOUND)
    add_executable(sdlplay
        "${CMAKE_CURRENT_SOURCE_DIR}/sdlplayer/sdlplay.c"
    )
    mikmod_example_common(sdlplay)

    target_include_directories(sdlplay PRIVATE ${SDL_INCLUDE_DIR})
    target_link_libraries(sdlplay PRIVATE ${SDL_LIBRARY})

    set_target_properties(sdlplay PROPERTIES OUTPUT_NAME "sdlplay")

    if(CMAKE_SYSTEM_NAME STREQUAL "DOS" OR CMAKE_C_COMPILER_ID STREQUAL "OpenWatcom")
        mikmod_example_deploy_dos(sdlplay)
    endif()
else()
    message(STATUS "SDL not found: sdlplayer example will not be built")
endif()

# ------------------------------------------------------------
# Summary
# ------------------------------------------------------------
message(STATUS "----------------------------------------")
message(STATUS "libmikmod examples configuration")
message(STATUS "  lib target      : ${MIKMOD_EXAMPLE_LIB_TARGET}")
message(STATUS "  examples bin dir: ${LIBMIKMOD_EXAMPLES_BIN_DIR}")
message(STATUS "  SDL found       : ${SDL_FOUND}")
message(STATUS "----------------------------------------")