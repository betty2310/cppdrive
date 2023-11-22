# This file specifies how the project should be built, using CMake.
# If you are unfamiliar with CMake, don't worry about all the details.
# The sections you might want to edit are marked as such, and
# the comments should hopefully make most of it clear.
#
# For many purposes, you may not need to change anything about this file.

cmake_minimum_required(VERSION 3.14)

# Set project name, version and laguages here.
project(CPPDRIVE VERSION 0.0.0.1 LANGUAGES CXX)


# Options: Things you can set via commandline options to cmake (e.g. -DENABLE_LTO=[ON|OFF])
option(ENABLE_WARNINGS_SETTINGS "Allow target_set_warnings to add flags and defines.
                                 Set this to OFF if you want to provide your own warning parameters." ON)
option(ENABLE_LTO "Enable link time optimization" ON)
option(ENABLE_DOCTESTS "Include tests in the library. Setting this to OFF will remove all doctest related code.
                        Tests in tests/*.cpp will still be enabled." OFF)

# Include stuff. No change needed.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(ConfigSafeGuards)
include(Colors)
include(LTO)
include(Warnings)

# Check for LTO support.
find_lto(CXX)

# --------------------------------------------------------------------------------
#                         Locate files (change as needed).
# --------------------------------------------------------------------------------
set(SOURCES          # All .cpp files in src/
    src/example.cpp
    src/account.cpp
)

set(LIBRARY_NAME engine)  # Default name for the library built from src/*.cpp (change if you wish)

# --------------------------------------------------------------------------------
#                            Build! (Change as needed)
# --------------------------------------------------------------------------------
# Compile all sources into a library.
add_library(${LIBRARY_NAME} OBJECT ${SOURCES})

# Lib needs its header files, and users of the library must also see these (PUBLIC). (No change needed)
target_include_directories(${LIBRARY_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Set the compile options you want (change as needed).
target_set_warnings(${LIBRARY_NAME} ENABLE ALL AS_ERROR ALL DISABLE Annoying)
# target_compile_options(${LIBRARY_NAME} ... )  # For setting manually.


add_executable(server app/server.cpp)   # Name of exec. and location of file.
target_link_libraries(server PRIVATE ${LIBRARY_NAME})  # Link the executable to library (if it uses it).
target_set_warnings(server ENABLE ALL AS_ERROR ALL DISABLE Annoying) # Set warnings (if needed).
target_enable_lto(server optimized)  # enable link-time-optimization if available for non-debug configurations
add_executable(client app/client.cpp)   # Name of exec. and location of file.
target_link_libraries(client PRIVATE ${LIBRARY_NAME})  # Link the executable to library (if it uses it).
target_set_warnings(client ENABLE ALL AS_ERROR ALL DISABLE Annoying) # Set warnings (if needed).
target_enable_lto(client optimized) 
# Set the properties you require, e.g. what C++ standard to use. Here applied to library and main (change as needed).
set_target_properties(
    ${LIBRARY_NAME} server client
      PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)
