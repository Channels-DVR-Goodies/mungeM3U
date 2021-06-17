//
// Created by paul on 12/28/20.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "buffer.h"

tBuffer * bufferNew( const char * p, size_t len )
{
    tBuffer * result = malloc(sizeof(tBuffer));

    result->pointer   = p;
    result->remaining = len;

    return result;
}

void bufferDropAnchor( tBuffer * buffer )
{
    buffer->anchor = buffer->pointer;
}

const char * bufferGetAnchorString( tBuffer * buffer )
{
    char * result = NULL;
    size_t len = buffer->pointer - buffer->anchor;
    if ( len > 0 )
    {
        result = malloc( len + 1 );
        if ( result != NULL)
        {
            memcpy( result, buffer->anchor, len );
            result[len] = '\0';
        }
    }
    return (const char *)result;
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
        && *buffer->pointer != '\"'
        && *buffer->pointer != '\n'
        && *buffer->pointer != '\r' )
    {
        buffer->pointer++;
        buffer->remaining--;
    }

    unsigned int len = buffer->pointer - start;

    /* Skip over trailing quote(s) */
    /* Sometimes entries in the M3U are malformed, and has two adjacent closing quotes */
    while ( buffer->remaining > 0
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

void bufferPrintToEOL( FILE * output, tBuffer * buffer )
{
    const char * end = buffer->pointer;
    size_t left = buffer->remaining;

    while ( *end != '\0'
         && *end != '\n'
         && *end != '\r'
         && left > 0 )
    {
        end++;
        left--;
    }
    size_t len = end - buffer->pointer;
    if (len > buffer->remaining)
    {
        len = buffer->remaining;
    }

    fputc( '\'', output );
    fwrite( buffer->pointer, len, 1, output );
    fputs( "\'\n", output );
}
