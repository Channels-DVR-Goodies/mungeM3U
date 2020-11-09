/**
   Copyright &copy; Paul Chambers, 2019.

   @ToDo Switch to UTF-8 string handling, rather than relying on ASCII backwards-compatibility
*/

#define _XOPEN_SOURCE 700
#include <features.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <libgen.h> // for basename()
#include <pwd.h>
#define __USE_MISC  // dirent.d_type is linux-specific, apparently
#include <dirent.h>
#define __USE_GNU
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <dlfcn.h>
#include <fcntl.h>
#include <stdbool.h>

#include "argtable3.h"
#include "mungeM3U.h"

#include "keywords.h"
#include "name.h"

const char * gExecutableName;
FILE *       gOutputFile;
int          gDebugLevel = 0;

typedef enum {
    rUnknown = 0,
    rMixed,
    rSD,
    rHD,
    rFHD,
    rUHD
} tResolution;


typedef struct {
    const char * name;
    unsigned int hash;
} tLineup;

typedef struct
{
    const char * name;
    unsigned int hash;
    tResolution  resolution;
} tGroup;

typedef struct {
    const char * url;
    tResolution  resolution;
} tFeed;

typedef struct
{
    const char * name;
    unsigned int hash;

    tFeed *      feed;
    tLineup *    lineup;
    tGroup *     group;
} tChannel;

tChannel * newChannel(void)
{
    return (tChannel *) calloc(1,sizeof(tChannel));
}

tChannel * freeChannel( tChannel * channel )
{
    if (channel != NULL)
    {
        if ( channel->feed != NULL)
        {
            free( channel->feed );
            channel->feed = NULL;
        }

        if ( channel->lineup != NULL)
        {
            free( channel->lineup );
            channel->lineup = NULL;
        }

        if ( channel->group != NULL)
        {
            free( channel->group );
            channel->group = NULL;
        }
        free( channel );
    }
    return NULL;
}

/*
 *
 */
unsigned int calcNameHash( const char * string )
{
    unsigned int hash = 0;
    const char * p = string;
    unsigned int c;

    while ( (c = gNameCharMap[ (unsigned char)*p++ ]) != '\0' )
    {
        switch ( c )
        {
        case kNameSeparator:
            /* ignore separators */
            break;

        default:
            hash = fNameHashChar( hash, c );
            break;
        }
    }
    return hash;
}


/**
 * trim any trailing whitespace from the end of the string
 *
 * @param line	line to be trimmed
 */
void trimTrailingWhitespace(char * line)
{
    char * t    = line;
    char * nwsp = line;

    if ( t != NULL )
    {
        while (*t != '\0')
        {
            if (!isspace(*t++))
            {
                // note: t has already been incremented
                nwsp = t;
            }
        }
        *nwsp = '\0';
    }
}


#define kEOF    255

static size_t length;
static const char * pointer;

void setBuf( size_t len, const char * p )
{
    length  = len;
    pointer = p;
}

unsigned char getBufChar( void )
{
    unsigned char result = kEOF;

    if ( length > 0 )
    {
        result = *pointer++;
        length--;
    }

    return result;
}

const char * getBufPos( void )
{
    return pointer;
}

size_t getBufRemaining( void )
{
    return length;
}

void skipEOLchars( void )
{
    while ( length > 0 )
    {
        if ( getKeywordWord( *pointer ) != kKeywordEOL )
        {
            break;
        }
        pointer++;
        length--;
    }
}

char * strdupToEOL( void )
{
    char       * result = NULL;
    const char * start  = getBufPos();
    unsigned int c;

    while ( getBufRemaining() > 0 )
    {
        c = getKeywordWord( getBufChar() );
        if ( c == kKeywordEOL || getBufRemaining() == 0 )
        {
            size_t len = getBufPos() - start;
            if ( c == kKeywordEOL )
            { --len; }
            result = calloc( 1, len + 1 );
            if ( result != NULL)
            {
                memcpy( result, start, len );
            }
            break;
        }
    }
    return result;
}


tChannel * processChannelName( const char * name )
{
    tChannel * result = NULL;
    char *        d;
    int           i;
    unsigned int  c;
    const char *  p = name;
    const char *  s = name;
    unsigned long hash = 0;
    char          temp[64];

    temp[0] = '\0';

    do {
        c = getNameWord( *p++ );
        switch ( c )
        {
        case '\0':
        case kNameSeparator:
            if ( hash != 0 )
            {
                switch ( hash )
                {
                case kNameVIP:
                    printf( "name: VIP\n" );
                    break;

                case kNameSD:
                    printf( "name: SD\n" );
                    break;

                case kNameHD:
                    printf( "name: 720p\n" );
                    break;

                case kNameFHD:
                    printf( "name: 1080p\n" );
                    break;

                default:
                    d = temp;
                    for ( i = sizeof( temp ); i > 0 && *d != '\0'; i-- )
                    { d++; }
                    while ( i > 0 && s < p )
                    {
                        *d++ = *s++;
                        i--;
                    }
                    *d = '\0';
                    break;
                }
                hash = 0;
            }
            s = p;
            break;

        default:
            hash = fNameHashChar( hash, c );
            break;
        }
    } while ( c != '\0' );
    trimTrailingWhitespace( temp );

    printf( "name: \"%s\"\n", temp );

    return result;
}

tGroup * processGroupName( const char * name )
{
    tGroup * result = NULL;
    const char * p     = name;
    const char * s     = name;
    char       * d;
    int           i;
    unsigned int  c;
    unsigned long hash = 0;
    char          temp[64];

    temp[0] = '\0';

    do {
        c = getNameWord( *p++ );
        switch ( c )
        {
        case '\0':
        case kNameSeparator:
            if ( hash != 0 )
            {
                switch ( hash )
                {
                case kNameVIP:
                    printf( "group: VIP\n" );
                    break;

                case kNameSD:
                    printf( "group: SD\n" );
                    break;

                case kNameHD:
                    printf( "group: 720p\n" );
                    break;

                case kNameFHD:
                    printf( "group: 1080p\n" );
                    break;

                case kNameHDMix:
                case kNameFHDMix:

                    printf( "group: HD Mix\n" );
                    break;

                default:
                    d       = temp;
                    for ( i = sizeof( temp ); i > 0 && *d != '\0'; i-- )
                    { d++; }
                    while ( i > 0 && s < p )
                    {
                        *d++ = *s++;
                        i--;
                    }
                    *d      = '\0';
                    break;
                }
                hash = 0;
            }
            s = p;
            break;

        default:
            hash = fNameHashChar( hash, c );
            break;
        }
    } while ( c != '\0' );
    trimTrailingWhitespace( temp );

    printf( "group: \"%s\"\n", temp );

    return result;
}

void processBuf( void )
{
    static unsigned long hash;
    static unsigned long lastKeyHash;

    while ( getBufRemaining() > 0 )
    {
        unsigned short w = getKeywordWord( getBufChar());
        switch ( w )
        {
        case kKeywordQuote:
            {
                char * str;
                const char * start = getBufPos();
                /* scan forward looking for the terminating quote */
                while ( getBufRemaining() > 0 )
                {
                    unsigned short w = getKeywordWord( getBufChar());
                    if ( w == kKeywordQuote )
                    {
                        size_t len = getBufPos() - start - 1;
                        str = calloc( 1, len + 1 );
                        if ( str != NULL)
                        {
                            memcpy( str, start, len );
                            switch ( lastKeyHash )
                            {
                            case kKeywordID:
                                printf( "       ID: \"%s\"\n", str );
                                break;

                            case kKeywordName:
                                printf( "     Name: \"%s\"\n", str );
                                processChannelName( str );
                                break;

                            case kKeywordLogo:
                                printf( "     Logo: \"%s\"\n", str );
                                break;

                            case kKeywordGroup:
                                printf( "    Group: \"%s\"\n", str );
                                processGroupName( str );
                                break;
                            }
                        }
                        hash = 0;
                        break;
                    }
                }
            }
            break;

        case kKeywordComma:
            {
                /* from here to EOL is also the name */
                char * name = strdupToEOL();
                printf( " trailing: \"%s\"\n", name );

                /* skip over end-of-line character(s) */
                skipEOLchars();

                /* the entire next line is the URL */
                char * url = strdupToEOL();
                printf( "      url: \"%s\"\n", url );

                /* skip over end-of-line character(s) */
                skipEOLchars();

                hash = 0;
            }
            break;

        case kKeywordAssign:
            if ( hash != 0 )
            {
                lastKeyHash = hash;
                hash        = 0;
            }
            break;

        case kKeywordSeparator: /* check hash for a known keyword */
            if ( hash == kKeywordEXTINF)
            {
                printf( "[Start]\n" );
            }
            hash = 0;
            break;

        default:
            hash = fKeywordHashChar( hash, w );
            break;
        }
    }
}

int processFile( const char * path )
{
    int result = 0;

    int fd = open( path, O_RDONLY );
    struct stat fileinfo;
    if ( fstat( fd, &fileinfo ) == -1 )
    {
        result = -errno;
    }
    else
    {
        size_t length = fileinfo.st_size;
        const char * map = (const char *) mmap( NULL, length, PROT_READ, MAP_PRIVATE, fd, 0 );
        setBuf( length, map );

        if ( map == MAP_FAILED || map == NULL )
        {
            result = -errno;
        }
        else
        {
            processBuf();
        }
    }
    return result;
}


const char * usage =
"Command Line Options\n"
"  -d <string>  set {destination} parameter\n"
"  -t <string>  set {template} paameter\n"
"  -x           pass each output string to the shell to execute\n"
"  --           read from stdin\n"
"  -0           stdin is null-terminated (also implies '--' option)\n"
"  -v <level>   set the level of verbosity (debug info)\n";

/* global arg_xxx structs */
static struct
{
    struct arg_lit  * help;
    struct arg_lit  * version;
    struct arg_str  * extn;
    struct arg_file * file;
    struct arg_end  * end;
} gOption;

int main( int argc, char * argv[] )
{
    int result = 0;

    gExecutableName = strrchr( argv[0], '/' );
    /* If we found a slash, increment past it. If there's no slash, point at the full argv[0] */
    if ( gExecutableName++ == NULL)
    { gExecutableName = argv[0]; }

    gOutputFile = stdout;

    /* the global arg_xxx structs above are initialised within the argtable */
    void * argtable[] =
    {
        gOption.help    = arg_litn( NULL, "help", 0, 1,
                                    "display this help (and exit)" ),
        gOption.version = arg_litn( NULL, "version", 0, 1,
                                    "display version info (and exit)" ),
        gOption.extn    = arg_strn( "x", "extension", "<extension>", 0, 1,
                                    "set the extension to use for output files" ),
        gOption.file    = arg_filen(NULL, NULL, "<file>", 1, 999,
                                    "input files" ),

        gOption.end     = arg_end( 20 )
    };

    int nerrors = arg_parse( argc, argv, argtable );

    if ( gOption.help->count > 0 )    /* special case: '--help' takes precedence over everything else */
    {
        fprintf( stdout, "Usage: %s", gExecutableName );
        arg_print_syntax( stdout, argtable, "\n" );
        fprintf( stdout, "process hash file into a header file.\n\n" );
        arg_print_glossary( stdout, argtable, "  %-25s %s\n" );
        fprintf( stdout, "\n" );

        result = 0;
    }
    else if ( gOption.version->count > 0 )   /* ditto for '--version' */
    {
        fprintf( stdout, "%s, version %s\n", gExecutableName, "(to do)" );
    }
    else if ( nerrors > 0 )    /* If the parser returned any errors then display them and exit */
    {
        /* Display the error details contained in the arg_end struct.*/
        arg_print_errors( stdout, gOption.end, gExecutableName );
        fprintf( stdout, "Try '%s --help' for more information.\n", gExecutableName );
        result = 1;
    }
    else
    {
        result = 0;
        int i = 0;

        gOutputFile = NULL;

        const char * extension = ".h";
        if ( gOption.extn->count != 0 )
        {
            extension = *gOption.extn->sval;
        }

        while ( i < gOption.file->count && result == 0 )
        {
            char output[FILENAME_MAX];
            strncpy( output, gOption.file->filename[i], sizeof( output ));


            char * p = strrchr( output, '.' );
            if ( p != NULL)
            {
                strncpy( p, extension, &output[sizeof( output ) - 1] - p );
            }

            gOutputFile = fopen( output, "w" );
            if ( gOutputFile == NULL)
            {
                fprintf( stderr, "### unable to open \'%s\' (%d: %s)\n",
                         output, errno, strerror(errno));
                result = errno;
            }

            if ( result == 0 )
            {
                result = processFile( gOption.file->filename[i] );
            }
            i++;

            fclose( gOutputFile );
        }
    }

    /* release each non-null entry in argtable[] */
    arg_freetable( argtable, sizeof( argtable ) / sizeof( argtable[0] ));

    return result;
}
