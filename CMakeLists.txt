cmake_minimum_required(VERSION 3.30)

project(GridironLite
  VERSION 0.1.0
  DESCRIPTION "Gridiron Lite - a small college football game"
  LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# Build configurations: Debug, Release and Profile.
#
#   Debug    - no optimisation, asserts on, full debug info.
#   Release  - optimised for size, LTO, no exceptions/RTTI. This is what ships.
#   Profile  - optimised like Release but keeps debug info and frame pointers
#              and skips LTO, so a profiler can still attribute samples.
#
# Profile is not a built-in CMake build type, so we register it ourselves.
# ---------------------------------------------------------------------------
set(GRIDIRON_BUILD_TYPES Debug Release Profile)

get_property(GRIDIRON_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(GRIDIRON_MULTI_CONFIG)
  # Visual Studio and friends choose the config at build time; make sure
  # Profile is offered alongside the built-in types.
  list(APPEND CMAKE_CONFIGURATION_TYPES ${GRIDIRON_BUILD_TYPES})
  list(REMOVE_DUPLICATES CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}"
      CACHE STRING "Available build configurations" FORCE)
else()
  # Single-config generators (Ninja, Make) take the type at configure time.
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build configuration" FORCE)
  endif()
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${GRIDIRON_BUILD_TYPES})
endif()

# Language standard is fixed for every target.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Emit compile_commands.json for clangd and other tooling.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Shared compiler/linker options live in cmake/.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(GridironFlags)

# ---------------------------------------------------------------------------
# Dependencies
#
# SDL3 is fetched and built from source rather than taken from the system, so
# every machine and CI job compiles the same pinned revision.  The download is
# pinned by SHA256, so a moved or tampered tag cannot silently change a build.
# Options are forced before MakeAvailable so SDL configures statically and
# without tests or examples, which keeps the size budget in check.
# ---------------------------------------------------------------------------
include(FetchContent)

set(GRIDIRON_SDL3_VERSION "3.4.18")

set(SDL_SHARED       OFF CACHE BOOL "" FORCE)
set(SDL_STATIC       ON  CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS        OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES     OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL      OFF CACHE BOOL "" FORCE)

FetchContent_Declare(SDL3
  URL "https://github.com/libsdl-org/SDL/releases/download/release-${GRIDIRON_SDL3_VERSION}/SDL3-${GRIDIRON_SDL3_VERSION}.tar.gz"
  URL_HASH "SHA256=9c75cf16330322c217dedd2e0609f1124f1b54b8633e763467b4684d0f4334a3"
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(SDL3)

# ---------------------------------------------------------------------------
# Targets
# ---------------------------------------------------------------------------
add_executable(GridironLite)
target_sources(GridironLite
  PRIVATE
    src/main.cpp
)
gridiron_apply_build_flags(GridironLite)

target_link_libraries(GridironLite
  PRIVATE
    SDL3::SDL3
)
