cmake_minimum_required(VERSION 3.10)
project(mungeM3U C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall -Wextra")

file(GLOB HASHES "*.hash")
set(HASH_HEADERS)
foreach (HASH ${HASHES})

    string(REGEX REPLACE "(.*).hash$" "\\1.h" HASH_HEADER ${HASH})

    add_custom_command(
            OUTPUT "${HASH_HEADER}"
            COMMAND hashstrings ${HASH}
            DEPENDS "${HASH}")

    set( HASH_HEADERS ${HASH_HEADERS} "${HASH_HEADER}" )

endforeach (HASH)

include_directories(.)

add_custom_target(hashes ALL DEPENDS ${OUTFILES})

add_executable( mungeM3U
                mungeM3U.c mungeM3U.h
                btree/btree.c btree/btree.h
                buffer.c buffer.h
                usstationdata.h
                ${HASH_HEADERS} )

target_link_libraries( mungeM3U argtable3 hashstrings m )

install( TARGETS mungeM3U
         RUNTIME DESTINATION /usr/bin )