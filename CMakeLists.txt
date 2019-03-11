cmake_minimum_required(VERSION 3.13)
project(supercal)

set(CMAKE_CXX_STANDARD 17)

add_executable(
        supercal
        src/main.cc

        include/xvariant.hh

        include/ical.hh           src/ical.cc
        include/icalstream.hh     src/icalstream.cc
        include/ICalParser.hh     src/IcalParser.cc
        include/parser_helpers.hh src/parser_helpers.cc
        include/rfc3629.hh        src/rfc3629.cc
        include/rfc3986.hh        src/rfc3986.cc
        include/rfc4288.hh        src/rfc4288.cc
        include/rfc5234.hh        src/rfc5234.cc
        include/rfc5646.hh        src/rfc5646.cc
)

include_directories(include/)


# ==============================================================================
# ==   Assets.   ===============================================================
# ==============================================================================
## -- dev-assets/ --------------------------------------------------------------
set(ASSET_FOLDER dev-assets)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/)
configure_file("${ASSET_FOLDER}/mini1.ics"
               ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/
               COPYONLY)
configure_file("${ASSET_FOLDER}/mini1.errors.ics"
               ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/
               COPYONLY)
## -- dev-assets/f1calendar.com/ -----------------------------------------------
set(ASSET_FOLDER dev-assets/f1calendar.com)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/)
configure_file("${ASSET_FOLDER}/2019_full.ics"
               ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/
               COPYONLY)
configure_file("${ASSET_FOLDER}/2019_full.ics.indented.txt"
               ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/
               COPYONLY)
