cmake_minimum_required(VERSION 3.10)
project(mungeM3U C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall -Wextra")

include_directories(.)

file(GLOB HASHES "*.hash")
set(OUTFILES)
foreach (HASH ${HASHES})

    string(REGEX REPLACE "(.*).hash$" "\\1.h" OUTPUT_FILE_NAME ${HASH})

    add_custom_command(
            OUTPUT "${OUTPUT_FILE_NAME}"
            COMMAND hashstrings ${HASH}
            DEPENDS "${HASH}")

    set(OUTFILES ${OUTFILES} "${OUTPUT_FILE_NAME}")

endforeach (HASH)

add_custom_target(hashes ALL DEPENDS ${OUTFILES})

add_executable( mungeM3U
                mungeM3U.c mungeM3U.h
                btree/btree.c btree/btree.h
                buffer.c buffer.h
                affiliate.h country.h fccdata.h genre.h
                keyword.h name.h nielsenDMA.h resolution.h
                usstate.h usstationdata.h city.h
                capitalization.h )

target_link_libraries(mungeM3U argtable3 hashstrings m)
