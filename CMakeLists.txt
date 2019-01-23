cmake_minimum_required(VERSION 3.13)
project(supercal)

set(CMAKE_CXX_STANDARD 17)

add_executable(
        supercal
        src/main.cc
)


# ==============================================================================
# ==   Assets.   ===============================================================
# ==============================================================================
## -- dev-assets/ --------------------------------------------------------------
set(ASSET_FOLDER dev-assets)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/)
## -- dev-assets/f1calendar.com/ -----------------------------------------------
set(ASSET_FOLDER dev-assets/f1calendar.com)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/)
configure_file("${ASSET_FOLDER}/2019_full.ics" ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/ COPYONLY)
