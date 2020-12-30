//
// Created by paul on 11/8/20.
//

#include "mungeFCC.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

const char * kPrefix = "struct FCCStationData[] = \n{\n";
const char * kSuffix = "};\n";

#define ServiceType( l1, l2, l3 ) (( l1 << 16) | (l2 << 8) | l3)

typedef enum
{
    kServiceUnSet = 0,
    kServiceLPA = ServiceType( 'L', 'P', 'A' ),
    kServiceLPD = ServiceType( 'L', 'P', 'D' ),
    kServiceLPT = ServiceType( 'L', 'P', 'T' ),
    kServiceLPX = ServiceType( 'L', 'P', 'X' )
} tServiceType;

typedef struct pStation {
    struct pStation * next;
    const char * callsign;
    tServiceType service;
    const char * rfChan;
    const char * logicalChan;
    const char * city;
    const char * state;
    const char * country;
} tStation;

int lenTrimmed( char * field, unsigned int length )
{
    int result = 0;
    char * p = field;

    if (length > 0 )
    {
        p += length - 1;
        while (  p > field && isspace( *p ) ) { --p; }
        result = (p - field) + 1;
    }
    return result;
}

void toTitleCase( char * title )
{
    int first = 1;
    char * p = title;
    while ( *p != '\0' )
    {
        if ( isspace(*p) || ispunct( *p ) )
        {
            first = 1;
        } else if (first) {
            *p = toupper(*p);
            first = 0;
        } else {
            *p = tolower(*p);
        }
        p++;
    }
}

tServiceType decodeService( char * service )
{
    unsigned char * p = (unsigned char *) service;
    unsigned int serviceType = *p++;
    serviceType = (serviceType << 8) | *p++;
    serviceType = (serviceType << 8) | *p;
    return (tServiceType)serviceType;
}

int processLine( char * line, tStation * station )
{
    int result = 0;
    char * p = line;
    char * q;
    int fieldNum;

    fieldNum = 1;
    while ( (q = strchr(p,'|')) != NULL )
    {
        int len = lenTrimmed( p, q - p );
        char * value = calloc( 1, len + 1 );
        if ( value != NULL)
        {
            memcpy( value, p, len );
            switch ( fieldNum )
            {
            case 2: // station callsign
                station->callsign = value;
                break;

            case 4: // station type
                station->service = decodeService( value );
                break;

            case 5: // RF channel
                station->rfChan = value;
                break;

            case 37: // logical channel
                station->logicalChan = value;
                break;

            case 11: // city
                toTitleCase( value );
                station->city = value;
                break;

            case 12: // state
                station->state = value;
                break;

            case 13: // country
                station->country = value;
                break;

            default:
                break;
            }
        }

        p = &q[1];
        ++fieldNum;
    }

    return result;
}

int main( int argc, const char *argv[] )
{
    int result = 0;
    char line[1024];
    tStation * head = NULL;
    tStation * station;

    while ( fgets(line, sizeof(line), stdin ) != NULL)
    {
        station = calloc( 1, sizeof(tStation) );
        if ( station != NULL ) {
            processLine( line, station );
            station->next = head;
            head = station;
        }
    }

    printf( "%s", kPrefix );
    for ( station = head; station != NULL; station = station->next ) {
        switch ( station->service )
        {
        case kServiceLPA:
        case kServiceLPD:
        case kServiceLPT:
        case kServiceLPX:
            break;

        default:
            fprintf( stdout, "\t%s, %c%c%c, %s, %s, %s,%s,%s\n",
                     station->callsign,
                     (station->service >> 16) & 0xff,
                     (station->service >> 8) & 0xff,
                     (station->service) & 0xff,
                     station->rfChan,
                     station->logicalChan,
                     station->city,
                     station->state,
                     station->country );
            break;
        }
    }
    printf( "%s", kSuffix );

    return result;
}