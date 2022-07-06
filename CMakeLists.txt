# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("LabNet")

# Include sub-projects.
add_subdirectory("LabNet")

add_subdirectory(vendor/sobjectizer/dev sobjectizer)

set(GATTLIB_BUILD_DOCS OFF)
add_subdirectory(vendor/gattlib gattlib)