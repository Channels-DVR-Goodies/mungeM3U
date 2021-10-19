cmake_minimum_required(VERSION 3.10)
project(mungeM3U C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall -Wextra")

set( HASH_HEADERS )
file( GLOB HASH_FILES "*.hash" )
foreach ( HASH_FILE ${HASH_FILES} )

    string( REGEX REPLACE "^(.*).hash$" "\\1.h" HASH_HEADER ${HASH_FILE} )

    add_custom_command( OUTPUT "${HASH_HEADER}"
                        DEPENDS "${HASH_FILE}"
                        COMMAND hashstrings ${HASH_FILE} )

    set( HASH_HEADERS ${HASH_HEADERS} "${HASH_HEADER}" )

endforeach ( HASH_FILE )

include_directories(.)

add_executable( mungeM3U
                mungeM3U.c mungeM3U.h
                btree/btree.c btree/btree.h
                buffer.c buffer.h
                usstationdata.h
                ${HASH_HEADERS} )

target_link_libraries( mungeM3U argtable3 hashstrings m )
