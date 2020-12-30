//
// Created by paul on 12/28/20.
//
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

tBuffer * bufferNew( const char * p, size_t len )
{
    tBuffer * result = malloc(sizeof(tBuffer));

    result->pointer   = p;
    result->remaining = len;

    return result;
}

unsigned char bufferGetChar( tBuffer * buffer )
{
    unsigned char result = '\0';

    if ( buffer->remaining > 0 )
    {
        result = *buffer->pointer++;
        buffer->remaining--;
    }

    return result;
}

size_t bufferGetRemaining( tBuffer * buffer )
{
    return buffer->remaining;
}

const char * bufferGetQuotedString( tBuffer * buffer )
{
    char * result = NULL;

    const char * start = buffer->pointer;

    while ( buffer->remaining > 0
        && *buffer->pointer != '\"' )
    {
        buffer->pointer++;
        buffer->remaining--;
    }

    unsigned int len = buffer->pointer - start;

    /* skip over the trailing quote */
    if ( buffer->remaining > 0
     && *buffer->pointer == '\"' )
    {
        buffer->pointer++;
        buffer->remaining--;
    }

    result = malloc( len + 1 );

    if ( result != NULL )
    {
        memcpy( result, start, len );
        /* always null-terminated */
        result[len] = '\0';
    }
    return (const char *)result;
}

const char * bufferGetStringToEOL( tBuffer * buffer )
{
    char * result;

    const char * start = buffer->pointer;

    while ( buffer->remaining > 0
        && *buffer->pointer != '\n'
        && *buffer->pointer != '\r' )
    {
        buffer->pointer++;
        buffer->remaining--;
    }

    unsigned int len = buffer->pointer - start;

    /* skip past any run of end-of-line characters */
    while ( buffer->remaining > 0
            && ( *buffer->pointer == '\n' || *buffer->pointer == '\r') )
    {
        buffer->pointer++;
        buffer->remaining--;
    }

    result = malloc( len + 1 );

    if ( result != NULL)
    {
        memcpy( result, start, len );
        /* always null-terminated */
        result[len] = '\0';
    }
    return (const char *)result;
}
