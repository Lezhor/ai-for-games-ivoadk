cmake_minimum_required(VERSION 3.28)

project(ivoadk_client C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(ENABLE_LTO       "Enable link-time optimization" OFF)
option(ENABLE_GUI       "Enable GUI using raylib" OFF)
option(ENABLE_TUI       "Enable TUI using ncurses" OFF)
option(ENABLE_GCOV      "Enable GCOV code coverage" OFF)

if(ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT got_ipo_support)
    if(got_ipo_support)
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

# --------------------------------------------------------
# Core Interface (Compiler Flags & Sanitizers)
# --------------------------------------------------------

add_library(core INTERFACE)
target_include_directories(core INTERFACE ${CMAKE_SOURCE_DIR}/src)
if(NOT MSVC)
    target_link_libraries(core INTERFACE m)
endif()

if(ENABLE_GCOV)
    target_compile_options(core INTERFACE --coverage)
    target_link_options(core INTERFACE --coverage)
endif()

# Compiler options
if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(core INTERFACE
        -Wall
        -Werror
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wint-conversion
        -Wfloat-conversion
        -fno-omit-frame-pointer)

    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten" AND APPLE)
        target_link_options(core INTERFACE -Wl,-dead_strip)
    endif()

    if (CMAKE_C_COMPILER_ID MATCHES "Clang" AND APPLE)
        target_compile_options(core INTERFACE
            -fcolor-diagnostics
            -ferror-limit=1000
            -Wimplicit-int-conversion
            -Wshorten-64-to-32)
    endif()
endif()

if (CMAKE_C_COMPILER_ID MATCHES "Clang" AND APPLE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(core INTERFACE
        -fsanitize=address
        -fsanitize=undefined)
    target_link_options(core INTERFACE
        -fsanitize=address
        -fsanitize=undefined)
endif()

if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(core INTERFACE /W4 /WX)
endif()

# --------------------------------------------------------
# Find Dependencies (GUI / TUI)
# --------------------------------------------------------

if(ENABLE_GUI)
    message(STATUS "GUI enabled")

    if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
        add_subdirectory(vendor/raylib)
    else()
        find_package(raylib CONFIG REQUIRED)
        find_package(glfw3 CONFIG REQUIRED)

        message(STATUS "Found raylib: ${raylib_DIR}")
        message(STATUS "Found glfw: ${glfw3_DIR}")

        # Upstream/vcpkg raylib config currently self-links libraylib.a via
        # INTERFACE_LINK_LIBRARIES. That yields duplicate raylib entries and
        # linker warnings ("Ignoring duplicate libraries: .../libraylib.a").
        # Workaround locally by clearing only the self-link interface.
        if(TARGET raylib)
            set_target_properties(raylib PROPERTIES INTERFACE_LINK_LIBRARIES "")
        endif()
    endif()
endif()


if(ENABLE_TUI)
    message(STATUS "TUI enabled")
    if(WIN32)
        find_package(unofficial-pdcurses CONFIG REQUIRED)
    else()
        set(CURSES_NEED_NCURSES TRUE)
        find_package(Curses REQUIRED)
        message(STATUS "Found ncurses: ${CURSES_LIBRARIES}")
    endif()
endif()

# --------------------------------------------------------
# Core Files
# --------------------------------------------------------
file(GLOB CORE_ROOT_FILES "${CMAKE_SOURCE_DIR}/src/*.c")
file(GLOB_RECURSE CORE_SUB_FILES
    "${CMAKE_SOURCE_DIR}/src/network/*.c"
    "${CMAKE_SOURCE_DIR}/src/utils/*.c"
    "${CMAKE_SOURCE_DIR}/src/game/*.c"
    "${CMAKE_SOURCE_DIR}/src/agent/core/*.c"
    "${CMAKE_SOURCE_DIR}/src/agent/logic/*.c"
    "${CMAKE_SOURCE_DIR}/src/agent/eval/minmax/*.c"
    "${CMAKE_SOURCE_DIR}/src/agent/eval/maxn/*.c"
)
set(CORE_FILES ${CORE_ROOT_FILES} ${CORE_SUB_FILES})

# --------------------------------------------------------
# Training Files
# --------------------------------------------------------
file(GLOB TRAIN_FILES "${CMAKE_SOURCE_DIR}/src/train/*.c")

# --------------------------------------------------------
# Agent Executables
# --------------------------------------------------------
function(add_agent_executable AGENT_NAME)
    set(TARGET_NAME "agent-${AGENT_NAME}")
    set(AGENT_FILE "${CMAKE_SOURCE_DIR}/src/agent/${AGENT_NAME}.c")

    # TODO: separately add GUI_FILES or TUI_FILES if booleans active

    # Compile the core simulation files alongside this specific agent file
    add_executable(${TARGET_NAME} ${CORE_FILES} ${AGENT_FILE})

    # Link the 'core' INTERFACE to apply our strict warnings and sanitizers
    target_link_libraries(${TARGET_NAME} PRIVATE core)

    # Set UI options for this agent
    target_compile_definitions(${TARGET_NAME} PRIVATE
        $<$<BOOL:${ENABLE_GUI}>:USE_GUI>
        $<$<BOOL:${ENABLE_TUI}>:USE_TUI>
    )

    # Link Raylib for GUIs
    if(ENABLE_GUI)
        target_link_libraries(${TARGET_NAME} PRIVATE raylib glfw)
        if(APPLE)
            target_link_libraries(${TARGET_NAME} PRIVATE
                "-framework Cocoa"
                "-framework OpenGL"
                "-framework IOKit"
                "-framework CoreVideo")
        endif()
    endif()

    # Link ncurses for TUI
    if(ENABLE_TUI)
        if(WIN32)
            target_link_libraries(${TARGET_NAME} PRIVATE unofficial::pdcurses::pdcurses)
        else()
            target_link_libraries(${TARGET_NAME} PRIVATE ${CURSES_LIBRARIES})
            target_include_directories(${TARGET_NAME} PRIVATE ${CURSES_INCLUDE_DIRS})
            target_compile_options(${TARGET_NAME} PRIVATE ${CURSES_CFLAGS})
        endif()
    endif()

    # Set Xcode working directory
    if(${CMAKE_GENERATOR} MATCHES "Xcode")
        set_target_properties(${TARGET_NAME} PROPERTIES
            XCODE_GENERATE_SCHEME TRUE
            XCODE_SCHEME_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    endif()

    message(STATUS "Agent executable created: ${TARGET_NAME}")
endfunction()

# --------------------------------------------------------
# Training Executables
# --------------------------------------------------------
function(add_train_executable AGENT_NAME)
    set(TARGET_NAME "train-${AGENT_NAME}")
    set(AGENT_FILE "${CMAKE_SOURCE_DIR}/src/agent/${AGENT_NAME}.c")

    add_executable(${TARGET_NAME} ${CORE_FILES} ${AGENT_FILE} ${TRAIN_FILES})
    target_link_libraries(${TARGET_NAME} PRIVATE core)

    # Inject the AGENT_TRAINING flag!
    target_compile_definitions(${TARGET_NAME} PRIVATE AGENT_TRAINING)

    message(STATUS "Train executable created: ${TARGET_NAME}")
endfunction()

# Register your Agents here!
add_agent_executable(idler)
add_agent_executable(randomizer)
add_agent_executable(minmax_hardcoded)
add_agent_executable(minmax_linear)
add_agent_executable(maxn)

add_train_executable(minmax_linear)

# TODO: add_agent_executable(human)


# --------------------------------------------------------
# Tests
# --------------------------------------------------------
enable_testing()

# Find testfiles in tests directory
file(GLOB_RECURSE TEST_FILES "${CMAKE_SOURCE_DIR}/tests/*_test.c")

set(ALL_TEST_TARGETS "")

foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

    # Compile the specific test file alongside all the core logic
    add_executable(${TEST_NAME} ${TEST_FILE} ${CORE_FILES})

    set_target_properties(${TEST_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")

    # Link the 'core' INTERFACE to get warnings/ASan in tests too
    target_link_libraries(${TEST_NAME} PRIVATE core)

    # Ensure tests can easily include files from src
    target_include_directories(${TEST_NAME} PRIVATE tests src)

    add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE:${TEST_NAME}>)
    list(APPEND ALL_TEST_TARGETS ${TEST_NAME})

    message(STATUS "Discovered test: ${TEST_NAME}")
endforeach()

if(ALL_TEST_TARGETS)
    list(LENGTH ALL_TEST_TARGETS TEST_COUNT)
    add_custom_target(all_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS ${ALL_TEST_TARGETS}
        COMMENT "Running all ${TEST_COUNT} tests..."
    )
endif()
