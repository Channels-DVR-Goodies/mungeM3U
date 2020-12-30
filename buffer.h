//
// Created by paul on 12/28/20.
//

#ifndef MUNGEM3U_BUFFER_H
#define MUNGEM3U_BUFFER_H

typedef struct {
    const char * pointer;
    size_t       remaining;
} tBuffer;

tBuffer * bufferNew( const char * p, size_t len);

unsigned char bufferGetChar( tBuffer * buffer );

size_t bufferGetRemaining( tBuffer * buffer );

const char * bufferGetQuotedString( tBuffer * buffer );

const char * bufferGetStringToEOL( tBuffer * buffer );

#endif //MUNGEM3U_BUFFER_H
