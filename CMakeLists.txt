cmake_minimum_required(VERSION 3.16)
project(adaptive_brute_force_mitigation VERSION 1.0.0 LANGUAGES CXX)

# ── C++ standard ──────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# ── Compiler warnings ─────────────────────────────────────────────────────────
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion)
endif()

# ── Build type default ────────────────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ── Source files ──────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/Account.cpp
    src/AttackLog.cpp
    src/DefenseManager.cpp
    src/Simulator.cpp
)

# ── Include paths ─────────────────────────────────────────────────────────────
include_directories(include)

# ── Executable ────────────────────────────────────────────────────────────────
add_executable(abfams ${SOURCES})

# ── Install rule (optional) ───────────────────────────────────────────────────
install(TARGETS abfams DESTINATION bin)

# ── CPack (optional packaging) ───────────────────────────────────────────────
set(CPACK_PACKAGE_NAME    "abfams")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
include(CPack)
