/**
   Copyright &copy; Paul Chambers, 2020.

   @ToDo Switch to UTF-8 string handling, rather than relying on ASCII backwards-compatibility
*/

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <dlfcn.h>
#include <fcntl.h>
#include <stdbool.h>

#include <argtable3.h>
#include <btree/btree.h>

#include <libhashstrings.h>

/* generated from hash files */
#include "capitalization.h"
#include "city.h"
#include "country.h"
#include "genre.h"
#include "keyword.h"
#include "name.h"
#include "nielsenDMA.h"
#include "resolution.h"
#include "uscallsign.h"
#include "usstate.h"

/* normal local includes */
#include "languages.h"
#include "countryCodes.h"
#include "usstationdata.h"

#define kHashExtMP4     0x003994c8
#define kHashExtMKV     0x003993a4
#define kHashExtAVI     0x003ddd89
#define kHashExtFLV     0x0039bac6
#define kHashExtM3U8    0x0983bd2a
#define kHashPeriod     0x0000002e


/* first pass parsing the line */

/*
 * these are common attributes that may occur in the channel and/or group name
 * so they are handled using a common structure & parsing. Attributes defined at
 * the group level are assumed to be inherited by every channel in the group,
 * unless the channel name itself has keywords to override that.
 */

typedef struct {
    const char *      originalName;
    const char *      name;
    bool              disabled;

    tResolutionIndex  resolution;
    bool              isVIP;
    bool              isPlus1;
    enum { kLinear = 0, kPPV, kVOD, k24x7, kLive } type;

    tCityIndex        city;
    tCountryIndex     country;
    tRegionIndex      region;
    tLanguage         language;
    tGenreIndex       genre;
    tAffiliateIndex   affiliate;
    tUSCallsignIndex  usStation;
} tCommon;

const char * lookupTypeAsString[] =
{
     [kLinear] = "linear",
     [kPPV]    = "Pay-Per-View",
     [kVOD]    = "Video On Demand",
     [k24x7]   = "24/7",
     [kLive]   = "Live Event"
};

typedef struct sStream {
    struct sStream *  next;
    const char     *  url;

    tResolutionIndex  resolution;
    bool              isVIP;
    bool              isFile;
} tStream;

typedef struct sChannel {
    tCommon           common;

	const char *      xui;
	const char *      id;
    const char *      logo;

    struct sStream *  stream;
    struct sGroup *   group;
} tChannel;

typedef struct sGroup {
    tCommon           common;
} tGroup;

struct {
    const char *       executableName;
    FILE *             outputFile;
    int                debugLevel;

    struct {
        struct btree * channel;
        struct btree * group;
    } tree;
    struct {
        tChannel *     channel;
        tGroup *       group;
        tStream *      stream;
    } head;
} global;

void dumpCommon(  FILE * output, tCommon  * common );
void dumpChannel( FILE * output, tChannel * channel );
void dumpGroup(   FILE * output, tGroup   * group );
void dumpStreams( FILE * output, tStream  * stream );


#define DEBUG_REJECTION

/**
 * @brief Decide if a group should be included in the output M3U
 *
 * This is the key function to modify to sort your needs.
 * It controls the filtering of channels imported from the original
 * M3U to produce the trimmed-down list of channels you actually
 * want to be in the output M3U.
 *
 * @param group
 * @return true if the group should be included
 */

bool isGroupDisabled( tGroup * group )
{
    bool result = false;
    tCommon * common = &group->common;

    switch ( common->genre )
    {
        /* Filter out genres I never record */
    case kGenreAdult:
    case kGenreCivic:
    case kGenreReligious:
    case kGenreShopping:
    case kGenreSports:
        result = true;
        break;

        /* the regional channels can bump up the channel count significantly.
         * since most methods of recording IPTV put a cap on the number of
         * channels that can be imported, removing redundant channels is worthwhile */
	case kGenreLocal:
	case kGenreCountry:
		switch ( common->country )
		{
		case kCountryUnitedStates:
			/* if we find a US Callsign, filter down to the SF Bay Area stations */
			if ( common->usStation != kUSCallsignUnset )
			{
			//	result = ( USStationData[ common->usStation ].stateIdx != kUSStateCalifornia );
			}
			break;

		case kCountryCanada:
			if ( common->city != kCityToronto && common->city != kCityVancouver )
			{
			//	result = true;
			}
			break;

		case kCountryUnitedKingdom:
			/* nuke the multitude of regional channels in the UK */
			if ( common->city != kCityLondon )
			{
			//	result = true;
			}
			break;

		default:
			result = true;
			break;
		}
		break;

    case kGenreUnset:
    default:
        break;
    }

    /* trim down to just UK and Canadian channels */
    if ( ! result )
    {
        switch ( common->country )
        {
        case kCountryUnset:
        case kCountryCanada:
        case kCountryUnitedKingdom:
            break;

        default:
            result = true;
            break;
        }
    }

    if ( ! result && (common->isPlus1 || common->type != kLinear ))
    {
        result = true;
    }

	if ( result )
	{
#ifdef DEBUG_REJECTION
		fprintf( stderr, "%10s ", lookupGenreAsString[ common->genre ] );
		dumpGroup( stderr, group );
#endif
	}

    return result;
}

/**
 * @brief Decide if a channel should be included in the output M3U
 *
 * This is the key function to modify to sort your needs.
 * It controls the filtering of channels imported from the original
 * M3U to produce the trimmed-down list of channels you actually
 * want to be in the output M3U.
 *
 * @param channel
 * @return true if the channel should be included
 */
bool isChannelDisabled( tChannel * channel )
{
    bool  result = false;
    tCommon * common = &channel->common;


    if ( channel->stream == NULL || channel->stream->isFile == true )
    {
        result = true;
#ifdef DEBUG_REJECTION
        fprintf( stderr, "      File ");
        dumpChannel( stderr, channel );
#endif
    }

    if ( ! result ) switch ( common->genre )
    {
        /* Filter out genres I never record */
    case kGenreAdult:
    case kGenreCivic:
    case kGenreReligious:
    case kGenreShopping:
    case kGenreSports:
        result = true;
#ifdef DEBUG_REJECTION
        fprintf( stderr, "     Genre ");
        dumpChannel( stderr, channel );
#endif
        break;

        /* The regional channels can bump up the channel count significantly. Since
         * most methods of recording IPTV put a cap on the number of channels that
         * can be imported, removing redundant channels is worthwhile */
    case kGenreLocal:
        switch ( channel->common.country )
        {
        case kCountryUnitedStates:
            /* if we find a US Callsign, filter down to the SF Bay Area stations */
            if ( common->usStation != kUSCallsignUnset )
            {
//                result = ( USStationData[ common->usStation ].nielsenDMAIdx != kNielsenDMASFBayArea );
            }
            break;

        case kCountryCanada:
            if ( common->city != kCityToronto && common->city != kCityVancouver )
            {
                result = true;
            }
            break;

        case kCountryUnitedKingdom:
            /* nuke the multitude of regional channels in the UK */
            if ( common->city != kCityLondon )
            {
                result = true;
            }
            break;

        default:
            result = true;
            break;
        }
#ifdef DEBUG_REJECTION
        if ( result == true )
        {
            fprintf( stderr, "     Local ");
            dumpChannel( stderr, channel );
        }
#endif
        break;

    case kGenreUnset:
    default:
        break;
    }

    /* trim down to just UK and Canadian channels */
    if ( ! result )
    {
        switch ( channel->common.country )
        {
        case kCountryUnset:
        case kCountryCanada:
		case kCountryUnitedKingdom:
		case kCountryUnitedStates:
            break;

        default:
            result = true;
#ifdef DEBUG_REJECTION
            fprintf( stderr, "   Country ");
            dumpChannel( stderr, channel );
#endif
            break;
        }
    }

    if ( ! result )
    {
        /* only channels that are in the english language */
        if ( common->language != kLanguageEnglish )
        {
            result = true;
#ifdef DEBUG_REJECTION
            fprintf( stderr, "  Language ");
            dumpChannel( stderr, channel );
#endif
        }
    }

    if ( ! result && (common->isPlus1 || common->type != kLinear ))
    {
        result = true;
#ifdef DEBUG_REJECTION
        fprintf( stderr, "      Live ");
        dumpChannel( stderr, channel );
#endif
    }

    if ( ! result)
    {
        if ( channel->stream != NULL && channel->stream->resolution == kResolutionSD )
        {
            result = true;
#ifdef DEBUG_REJECTION
            fprintf( stderr, "Resolution ");
            dumpChannel( stderr, channel );
#endif
        }
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static inline int min( int a, int b )
{
    if ( a < b )
         return a;
    else return b;
}

static inline int bound( int value, int lower, int upper )
{
    if ( value < lower ) return lower;
    if ( value > upper ) return upper;
    return value;
}
/* Uglyness because of inconsistent formatting of '+1' channels.
 * Some lack a separator between the channel name and the '+1'
 * this code detects that case and inserts a space */
/**
 * @brief
 * @param str
 * @return
 */
char * seperatePlus1( char * str )
{
    char * s = strrchr( str, '+' );

    /* if a '+' was found, and it is followed by a '1', but NOT preceded by a ' '...  */
    if ( s != NULL && s[ 1 ] == '1' && s[ -1 ] != ' ' )
    {
        /* enlarge the string to make room for the extra char */
        str = realloc( (void *)str, strlen( str ) + 2 );

        /* find the end of the string */
        while ( *s != '\0' ) { s++; }

        /* reverse back up the string, moving existing chars
         * up by one until we hit the plus we found earlier */
        do {
            s[ 1 ] = s[ 0 ];
        } while ( *s-- != '+' );

        s[1] = ' '; /* make sure there's a separator before the plus */
    }
    return str;
}

/**
 * @brief
 * @param p
 */
void trimTrailingEOL( char * p )
{
    char * start = p;
    /* find the end of the string */
    while ( *p != '\0' ) { ++p; }
    /* back up before the nul */
    --p;
    /* reverse over any trailing newlines/carriage returns */
    while ( start < p && ( *p == '\n' || *p == '\r' ) ) { --p; }
    p[1] = '\0';
}

void trimAppendQuote( char * p )
{
	char * start = p;
	/* find the end of the string */
	while ( *p != '\0' ) { ++p; }
	/* back up before the nul */
	--p;
	/* reverse over any trailing newlines/carriage returns */
	while ( start < p && ( *p == '\n' || *p == '\r' ) ) { --p; }
	p[1] = '\"';
	p[2] = '\0';
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* in hindsight, I probably should have used C++ instead... */
/**
 * @brief
 * @return
 */
tStream * newStream( void )
{
    return (tStream *) calloc( 1, sizeof( tStream ));
}

/**
 * @brief
 * @param stream
 */
void freeStream( tStream * stream )
{
    free( (void *)stream->url );
    free( (void *)stream );
}

/**
 * @brief
 * @return
 */
tChannel * newChannel(void)
{
    return (tChannel *) calloc(1,sizeof( tChannel ));
}

/**
 * @brief
 * @param channel
 * @return
 */
tChannel * freeChannel( tChannel * channel )
{
    if (channel != NULL)
    {
        tStream * stream = channel->stream;
        channel->stream = NULL;
        while ( stream != NULL )
        {
            tStream * next = stream->next;
            freeStream( stream );
            stream = next;
        }

        free( channel );
    }
    return NULL;
}

/**
 * @brief
 * @return
 */
tGroup * newGroup( void )
{
    return (tGroup *) calloc( 1, sizeof( tGroup ) );
}

/**
 * @brief
 * @param group
 */
void freeGroup( tGroup * group )
{
    if (group != NULL)
    {
        free( (void *) group->common.name );
        free( (void *) group );
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * @brief
 * @param output
 * @param common
 */
void dumpCommon( FILE * output, tCommon * common)
{
    if ( common->genre != kGenreUnset )
    {
        fprintf( output, ", genre: %s", lookupGenreAsString[ common->genre ] );
    }
    if ( common->usStation != kUSCallsignUnset )
    {
        fprintf( output, ", callsign: \"%s\"",
                 USStationData[ common->usStation ].callsign );
        fprintf( output, ", state: %s",
                 lookupUSStateAsString[ USStationData[ common->usStation ].stateIdx ] );
        fprintf( output, ", DMA: \"%s\"",
                 lookupNielsenDMAAsString[ USStationData[ common->usStation ].nielsenDMAIdx ] );
    }
    if ( common->affiliate != kAffiliateUnset )
    {
        fprintf( output, ", affiliate: %s", lookupAffiliateAsString[ common->affiliate ] );
    }
    if ( common->region != kRegionUnset )
    {
        fprintf( output, ", region: %s", lookupRegionAsString[ common->region ] );
    }
    if ( common->language != kLanguageUnset )
    {
        fprintf( output, ", language: %s", lookupLanguageAsString[ common->language ] );
    }

    if ( common->resolution != kResolutionUnset )
    {
        fprintf( output, ", resolution: %s", lookupResolutionAsString[ common->resolution ] );
    }
    if ( common->isVIP )
    {
        fprintf( output, ", VIP" );
    }

    if (common->isPlus1)
    {
        fprintf(output,", +1");
    }
}

/**
 * @brief
 * @param output
 * @param stream
 */
void dumpStreams( FILE * output, tStream * stream )
{
    while ( stream != NULL )
    {
        fprintf( output, "   stream: rez: %s, isvip: %d, url: %s\n",
                 lookupResolutionAsString[ stream->resolution ],
                 stream->isVIP,
                 stream->url );
        stream = stream->next;
    }
}

/**
 * @brief
 * @param output
 * @param channel
 */
void dumpChannel( FILE * output, tChannel * channel )
{
    fprintf( output, "  channel " );
    if ( channel->common.name != NULL )
    {
        fprintf( output, "name: %s", channel->common.name );
    }
    if ( channel->common.country != kCountryUnset )
    {
        fprintf( output, ", country: %s", lookupFullCountryAsString[ channel->common.country ] );
        if ( channel->common.country == kCountryCanada && channel->common.city != kCityUnset )
        {
            fprintf( output, ", city: %s", lookupCityAsString[ channel->common.city ] );
        }
    }
    dumpCommon( output, &channel->common );

    fprintf( output, ".\n" );
}

/**
 * @brief
 * @param output
 * @param group
 */
void dumpGroup( FILE * output, tGroup * group )
{
    fprintf( output, "    group " );
    if ( group->common.name != NULL )
    {
        fprintf( output, "name: %s", group->common.name );
    }
    if ( group->common.country != kCountryUnset )
    {
        fprintf( output, ", country: %s", lookupFullCountryAsString[group->common.country] );
        if ( group->common.country == kCountryCanada && group->common.city != kCityUnset )
        {
            fprintf( output, ", city: %s", lookupCityAsString[group->common.city] );
        }
    }
    dumpCommon( output, &group->common );

    fprintf( output, ".\n" );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * @brief
 * @param skipTable
 * @param hash
 * @param setting
 * @return
 */
bool assignHash( tRecord skipTable[], tHash hash, tIndex * setting )
{
    tIndex index = findHash( skipTable, hash );
    if ( index != kIndexUnset && *setting == kIndexUnset )
    {
        *setting = index;
    }
    return (index != kIndexUnset);
}

/**
 * @brief
 * @param hash
 * @param common
 * @return true - swallow the string
 */
bool processCommonHash( tHash hash, tCommon * common )
{
    bool swallow = false;

    if ( common->country == kCountryUnset && assignHash( mapCountrySearch, hash, &common->country ) )
    {
        swallow = true;
        if ( common->region == kRegionUnset )
        {
            common->region = countryToRegion[ common->country ];
        }

        if ( common->language == kLanguageUnset )
        {
            common->language = countryToLanguage[ common->country ];
        }
    }

    switch (findHash( mapNameSearch, hash ))
    {
    case kNameVIP:
        common->isVIP = true;
        swallow = true;
        break;

    case kNamePlus1:
        common->isPlus1 = true;
        break;

    case kNamePayPerView:
        common->type = kPPV;
        break;

    case kName24x7:
        common->type = k24x7;
        break;

    case kNameVideoOnDemand:
        common->type = kVOD;
        break;

    case kNameLiveEvent:
        common->type = kLive;
        break;

    case kNameFrenchCanadian:
        common->language = kLanguageFrench;
        break;

    case kNameLatino:
        common->language = kLanguageSpanish;
        break;
    }

    if ( assignHash( mapResolutionSearch, hash, &common->resolution ) )
    {
        swallow = true;
    }

    if ( common->country == kCountryUnitedStates || common->country == kCountryCanada )
    {
        assignHash( mapUSCallsignSearch, hash, &common->usStation );
        if ( common->usStation != kUSCallsignUnset )
        {
            common->country   = kCountryUnitedStates;
            common->genre     = kGenreLocal;
            common->affiliate = USStationData[ common->usStation ].affiliateIdx;
        }
    }

    if ( common->genre == kGenreUnset && assignHash( mapCitySearch, hash, &common->city ) )
    {
        common->genre = kGenreLocal;
    }

    /* if a US Station callsign was already found, then genre has already been set to 'regional' */

    /* Allow a second genre to override 'sports', since group names of 'sports and entertainment' are common*/
    if ( common->genre == kGenreUnset || common->genre == kGenreSports )
    {
        assignHash( mapGenreSearch, hash, &common->genre );
    }

    /* This is a stronger indication of a station's language than the country, e.g. hispanic networks in the U.S. */
    if (common->affiliate != kAffiliateUnset)
    {
        common->language  = affiliateToLanguage[ common->affiliate ];
    }

    return swallow;
}


/**
 * @brief
 * @param stream
 * @param channel
 */
void inheritChannel( tStream * stream, tChannel * channel )
{
    /* copy over any unset attributes from channel to stream */
    if ( stream != NULL && channel != NULL )
    {
        /* inherit resolution from channel if stream resolution is not set */
        if ( stream->resolution == kResolutionUnset )
        {
            stream->resolution = channel->common.resolution;
        }

        /* inherit stream VIP status from channel if not already VIP */
        if ( channel->common.isVIP )
        {
            stream->isVIP = channel->common.isVIP;
        }
    }
}

/**
 * @brief
 * @param channel
 * @param group
 */
void inheritGroup( tChannel * channel, tGroup * group )
{
    if ( channel != NULL && group != NULL )
    {
        /* copy over any unset attributes from group to channel */
        if ( channel->common.country == kCountryUnset
            && group->common.country != kCountryUnset )
        {
            channel->common.country = group->common.country;
        }

        if ( channel->common.language == kLanguageUnset
            && group->common.language != kLanguageUnset )
        {
            channel->common.language = group->common.language;
        }

        if ( channel->common.genre == kGenreUnset
            && group->common.genre != kGenreUnset )
        {
            channel->common.genre = group->common.genre;
        }

        if ( channel->common.affiliate == kAffiliateUnset
            && group->common.affiliate != kAffiliateUnset )
        {
            channel->common.affiliate = group->common.affiliate;
        }

        /* inherit resolution from group if channel resolution is not set */
        if ( channel->common.resolution == kResolutionUnset )
        {
            channel->common.resolution = group->common.resolution;
        }

        if ( channel->common.type == kLinear
            && group->common.type != kLinear )
        {
            channel->common.type = group->common.type;
        }

        if ( group->common.isVIP )
        {
            channel->common.isVIP = true;
        }
    }
}

/**
 * @brief
 * @param url
 * @param channel
 * @return
 */
tStream * processStream( const char * url, tChannel * channel )
{
    tStream * stream = newStream();
    if ( stream != NULL)
    {
        stream->url = url;
        inheritChannel( stream, channel );

        if ( url != NULL)
        {
            const char * ext;
            for ( ext = url; *ext != '\0'; ++ext ) { /* find the end-of-string */ }
            for ( int i = 5; ext > url && *ext != '.' && i > 0; --i )
            {
                /* scan backwards up to 5 characters, looking for a period */
                --ext;
            }

            if ( *ext == '.' )
            {
                tHash hash = hashString( ext, gNameCharMap );
                switch ( hash )
                {
                case kHashExtMP4:
                case kHashExtMKV:
                case kHashExtAVI:
                case kHashExtFLV:
                case kHashExtM3U8:
                    stream->isFile = true;
                    break;

                case kHashPeriod: /* some URLs have trailing periods? */
                    break;

                default:
                    if ( hash != 0 ) {
                        fprintf( stderr, "%s = 0x%08lx (%s)\n", ext, hash, url );
                    }
                    break;
                }
            }
        }

        // dumpChannel( stderr, channel );
    }

    return stream;
}

/**
 * @brief
 * @param name
 * @param common
 */
void processName( const char * name, tCommon * common )
{
    tMappedChar   mappedC;
    const char *  p;
    char *        sp;
    char *        dp;
    int           dl;
    char          temp[256];

    /* first extract any attributes embedded in the channel name,
     * tags like 'VIP', 'UK', 'HD', etc. See name.hash */

    common->originalName = strdup( name );
    p = name;
    if ( p == NULL || strlen(p) == 0 )
    {
        common->name = strdup( "(none)" );
        return;
    }

    sp = temp;
    dp = sp;

    dp[0] = '\0';
    dl = sizeof( temp ) - 1;

    tHash hash = 0;
    bool first = true;
    int alpha = 0;
    do {
        mappedC = remapChar( gNameCharMap, *p++ );
        if ( mappedC != kNameSeparator && mappedC != '\0' )
        {
            hash = hashChar( hash, mappedC );
            *dp++ = mappedC;
            if ( isalpha( mappedC ) )
            {
                ++alpha;
            }
        }
        else
        {
            if ( hash != 0 )
            {
                if ( processCommonHash( hash, common ) )
                {
                    /* swallow the string - back up to the beginning of this hash run */
                    dp = sp;
                }
                else
                {
                    tCapitalizationIndex capitalize = findHash( mapCapitalizationSearch, hash );
                    if ( capitalize != kCapitalizationUnset )
                    {
                        const char * p = lookupCapitalizationAsString[ capitalize ];
                        dp = stpcpy( sp, p );
                        if ( mappedC != '\0' )
                        {
                            *dp++ = ' ';
                        }
                        sp = dp;
                    }
                    else
                    {
                        for ( char * q = sp; q < dp; q++ )
                        {
                            if ( first )
                            {
                                *q = toupper( *q );
                            }
                            if ( isalpha( *q ) && alpha > 3 )
                            {
                                first = false;
                            }
                        }
                        /* if we're not at the end of the string, add a separator */
                        if ( mappedC != '\0' )
                        {
                            *dp++ = ' ';
                        }
                        /* remember the start of the next hash run */
                        sp = dp;
                    }
                }
                *dp = '\0';

                hash = 0;
            }
            first = true;
            alpha = 0;
        }
    } while ( mappedC != '\0' );

    /* nuke the trailing space, if there is one */
    if ( dp[-1] == ' ' )
    {
        *(--dp) = '\0';
        dl++;
    }

    /* if we extracted a country, append it to the name */
    if ( common->country != kCountryUnset )
    {
        int len = strlen( temp );
        if ( len != 0 )
        {
            snprintf( &temp[ len ], sizeof( temp ) - len, " (%s)", lookupCountryAsString[ common->country ] );
        }
        else
        {
            const char * fullCountry = lookupFullCountryAsString[ common->country ];
            if ( fullCountry != NULL )
            {
                strncpy( temp, fullCountry, sizeof(temp) );
                if ( common->genre == kGenreUnset )
                {
                    common->genre = kGenreCountry;
                }
            }
        }
    }
    common->name = strdup( temp );
}

/**
 * @brief
 * @param stream
 * @param name
 * @return
 */
tChannel * processChannelName( const char * name, tGroup * group )
{
    tChannel * channel = newChannel();
    if ( channel != NULL )
    {
        // fprintf( stderr, "channel: %p name: %s\n", channel, name );
        channel->group = group;
        processName( name, &channel->common );
        inheritGroup( channel, group );

        /* Let's see if we already have a matching channel */
        tChannel ** chan = btree_get( global.tree.channel, &channel );

        if ( chan == NULL )
        {
            if ( btree_set( global.tree.channel, &channel ) == NULL )
            {
                if  ( btree_oom( global.tree.channel ) )
                {
//                    fprintf( stderr, "channel btree_set() failed - out of memory\n" );
                }
                else {
//                    fprintf( stderr, "new channel added: %s\n", channel->common.name );
                }
            }
            else
            {
                fprintf( stderr, "### error: channel replaced: %s\n", channel->common.name );
            }
        }
        else
        {
            /* group already exists, so discard the local one */
            freeChannel( channel );
            /* and switch to the existing one */
            channel = *chan;
//            fprintf( stderr, "existing channel: %s\n", channel->common.name );
        }
    }

    return channel;
}

/**
 * @brief
 * @param name
 * @return
 */
tGroup * processGroupName( const char * name )
{
    tGroup * group = newGroup();
    if ( group != NULL )
    {
        // fprintf( stderr, "  group: %p name: %s\n", group, name );
        processName( name, &group->common );

        /* Let's see if we already have a matching group */
        tGroup ** grp = btree_get( global.tree.group, &group );

        if ( grp != NULL)
        {
            /* group already exists, so discard the local one */
            freeGroup( group );
            /* and switch to the existing one */
            group = *grp;
//            fprintf( stderr, "group exists: %s\n", group->common.name );
        }
        else
        {
            /* didn't find it, so add new group */
            if ( btree_set( global.tree.group, &group ) == NULL)
            {
                if  ( btree_oom( global.tree.group ) )
                {
                    fprintf( stderr, "group btree_set() failed - out of memory\n" );
                }
                else {
//                    fprintf( stderr, "new group added: %s\n", group->common.name );
                }
            }
            else
            {
                fprintf( stderr, "### error: group replaced: %s\n", group->common.name );
            }
        }
    }

    return group;
}


/**
 * @brief
 * @param inputFile
 * @return
 */
void importM3Uentry( const char * p )
{
	const char * valueStart;
	char * value;

	const char * xui_id 	 = NULL;
	const char * tvg_id 	 = NULL;
	const char * tvg_name 	 = NULL;
	const char * tvg_logo 	 = NULL;
	const char * group_title = NULL;
	const char * url 		 = NULL;

	tGroup   * group   = NULL;
	tChannel * channel = NULL;
	tStream  * stream  = NULL;

	tHash hash = 0;
	for ( ; *p != '\0'; ++p )
	{
		tMappedChar w = remapChar( gKeywordCharMap, *p );
		switch ( w )
		{
		case kKeywordSeparator:
			hash = 0;
			break;

		case kKeywordAssign:
			++p; /* skip over the equals sign */
			/* skip over the leading double-quote */
			if ( *p == '"' )
			{ ++p; }
			valueStart = p;

			/* fast-forward to either next tag, or end-of-string */
			while ( *p != '=' && *p != '\0' )
			{ ++p; }

			if ( *p == '=')
			{
				/* back up to the separator before the tag name */
				while ( p > valueStart && *p != ' ' )
				{ --p; }
			}
			/* back up over the trailing double-quote */
			while ( p > valueStart && *p != '"' )
			{ --p; }
			value = strndup( valueStart, p - valueStart );

			/* convert any double-quotes embedded in the string to single-quotes.
			 * Yep, I've actually seen this in the wild, even though that's clearly invalid. */
			for ( char * q = value; *q != '\0'; q++ )
			{
				if ( *q == '"' )
				{ *q = '\''; }
			}

			// fprintf( stderr, "value: %s\n", value );
			switch ( findHash( mapKeywordSearch, hash ))
			{
			case kKeywordXUI:
				xui_id = value;
//				fprintf( stderr, "xui_id: \'%s\'\n", xui_id );
				break;

			case kKeywordID:
				tvg_id = value;
//				fprintf( stderr, "tvg_id: \'%s\'\n", tvg_id );
				break;

			case kKeywordName:
				tvg_name = seperatePlus1((char *)value );
//				fprintf( stderr, "tvg_name: \'%s\'\n", tvg_name );
				break;

			case kKeywordLogo:
				tvg_logo = value;
//				fprintf( stderr, "tvg_logo: \'%s\'\n", tvg_logo );
				break;

			case kKeywordGroup:
				group_title = value;
//				fprintf( stderr, "group_title: \'%s\'\n", group_title );
				break;

			case kKeywordURL:
				url = value;
//				fprintf( stderr, "url: \'%s\'\n", url );
				break;

			default:
				fprintf( stderr, "Warning: unknown field\n" );
				break;
			}
			// fprintf( stderr, "%12s = (%s)\n", lookupKeywordAsString[ keyword ], value );

			hash = 0;
			break;

		default:
			hash = hashChar( hash, w );
			break;
		}
	}

	/* post-process the fields, now that we have collected them all */
	if ( group_title != NULL)
	{
		group = processGroupName( group_title );
	}
	if ( tvg_name != NULL)
	{
		channel = processChannelName( tvg_name, group );
		if ( channel != NULL)
		{
			channel->xui  = xui_id;
			channel->id   = tvg_id;
			channel->logo = tvg_logo;
			if ( group != NULL )
			{
				channel->group = group;
			}
		}
	}
	if ( channel != NULL && url != NULL)
	{
		stream = processStream( url, channel );
		/* insertion sort keeps higher resolution streams earlier in the list */
		tStream  * strm;
		tStream ** prev = &channel->stream;
		for ( strm = channel->stream; strm != NULL; strm = strm->next )
		{
			if ( stream->resolution > strm->resolution )
			{
				stream->next = strm;
				break;
			}
			prev = &strm->next;
		}
		*prev = stream;
	}
	if ( group != NULL )
	{
		group->common.disabled = isGroupDisabled( group );
	}
	if ( channel != NULL )
	{
		channel->common.disabled = isChannelDisabled( channel );
	}
}

/**
 * @brief parse the M3U file into a linked list of tEntry structures
 * @param inputFile
 */
int importM3U( FILE * inputFile )
{
	int result = 0;
	static char buffer[32768];
	char * p;

    while ( (p = fgets( buffer, sizeof( buffer ), inputFile )) != NULL && result == 0 )
    {
		trimTrailingEOL( p );
		if ( strncmp( p, "#EXTINF:-1 ", 11 ) == 0 )
		{
			/* skip over the leading '#EXTINF:-1 ` */
			p += 11;
			char * q = strrchr( p, ',' );
			if (q != NULL && *(q-1) == '"' )
			{
				*q = '\0';
			}

			strncat( buffer, " x-url=\"", sizeof( buffer ) - 1 );

			int    l = strlen(p);

			char * url = fgets( &p[l], sizeof( buffer ) - l, inputFile );
			if ( url != NULL)
			{
				trimAppendQuote( &p[ l ] );
				importM3Uentry( p );
			} else
			{
				result = errno;
				fprintf( stderr, "### read failure (%d: %s)\n", errno, strerror(errno));
			}
		}
		fprintf( stderr, "line: %s\n", p );
	}

	/* fgets failed, see if there was an error */
	if ( errno != 0 )
	{
		result = errno;
		fprintf( stderr, "### read failure (%d: %s)\n", errno, strerror(errno));
	}

	return result;
}

/**
 * @brief
 * @param output
 * @param channel
 */
void exportChannel( FILE * output, tChannel * channel )
{
#if 0
    dumpGroup( output, channel->group );
    dumpChannel( output, channel );
    dumpStreams( output, channel->stream );
#else

#if 0
    char    country[20];

    country[0] = '\0';
    if ( channel->common.country != kCountryUnset )
    {
        snprintf( country, sizeof(country), " (%s)", lookupCountryAsString[ channel->common.country ] );
    }
#endif

    char resolution[20];

    resolution[0] = '\0';
    if ( channel->common.resolution != kResolutionUnset )
    {
        snprintf( resolution, sizeof(resolution), " [%s]", lookupResolutionAsString[ channel->common.resolution ] );
    }

    fprintf( output, "#EXTINF:-1 xui_id=\"%s\" tvg-id=\"%s\" tvg-name=\"%s%s\" tvg-logo=\"%s\" group-title=\"%s\",%s%s\n%s\n",
			 channel->xui,
			 channel->id,
             channel->common.name, resolution,
             channel->logo,
             channel->group->common.name,
             channel->common.name, resolution,
             channel->stream->url );
#endif
}


bool interateChannel( const void * item, void * udata )
{
    (void)udata;

    tChannel * channel = *(tChannel **)item;

    // fprintf( stderr, "%d %d %s\n", channel->common.disabled, channel->group->common.disabled, channel->common.name );
    if (! channel->common.disabled && ! channel->group->common.disabled )
    {
        exportChannel( stdout, channel );
    }

    return true;
}

bool interateGroup( const void * item, void * udata )
{
    (void)udata;

    tGroup * group = *(tGroup **)item;

    fprintf( stderr, "%d \'%s\',\'%s\' [%s] (%s)\n",
             group->common.disabled,
             group->common.originalName,
             group->common.name,
             lookupGenreAsString[ group->common.genre ],
             lookupTypeAsString[ group->common.type ] );

    return true;
}

/**
 * @brief
 * @param output
 */
void exportM3U( FILE * output )
{
    fprintf( output, "#EXTM3U\n" );

    btree_ascend( global.tree.channel, NULL, interateChannel, NULL );
    //btree_ascend( global.tree.group,   NULL, interateGroup,   NULL );
}

/**
 * @brief
 * @param left
 * @param right
 * @param udata
 * @return
 */
int compareChannels( const void *left, const void *right, void *udata )
{
    int result;

    (void)udata;

    const tChannel * l = *(const tChannel **)left;
    const tChannel * r = *(const tChannel **)right;

    result = bound( l->common.country - r->common.country, -1, 1 );

    if ( result == 0 )
    {
        result = strcmp( l->common.name, r->common.name );
    }

    // fprintf( stderr, "channel: left: %s, right: %s, result %d\n", l->common.name, r->common.name, result );
    return result;
}

/**
 * @brief
 * @param left
 * @param right
 * @param udata
 * @return
 */
int compareGroups( const void * left, const void * right, void * udata )
{
    int result;

    (void)udata;

    const tGroup * l = *(const tGroup **)left;
    const tGroup * r = *(const tGroup **)right;

    result = bound( l->common.country - r->common.country, -1, 1 );
    if ( result == 0 )
    {
        result = strcmp( l->common.name, r->common.name );
    }

    // fprintf( stderr, "group: left: %s, right: %s, result %d\n", l->common.name, r->common.name, result );
    return result;
}


/**
 * @brief
 * @param path
 * @return
 */
int processFile( const char * path )
{
    int result = 0;

    FILE * inputFile = fopen( path, "r" );

    if ( inputFile == NULL )
    {
        result = -errno;
        fprintf( stderr, "### unable to open \'%s\' (%d: %s)\n", path, errno, strerror(errno) );
    }
    else
    {
        global.tree.channel = btree_new( sizeof( tChannel * ), 0, compareChannels, NULL );
        global.tree.group   = btree_new( sizeof( tGroup * ),   0, compareGroups,   NULL );

        importM3U( inputFile );
        exportM3U( stdout );

        btree_free( global.tree.channel );
        btree_free( global.tree.group );
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

    global.head.group   = NULL;
    global.head.channel = NULL;
    global.head.stream  = NULL;

    global.executableName = strrchr( argv[0], '/' );
    /* If we found a slash, increment past it. If there's no slash, point at the full argv[0] */
    if ( global.executableName++ == NULL)
    {
        global.executableName = argv[0];
    }

    global.outputFile = stdout;

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
        fprintf( stdout, "Usage: %s", global.executableName );
        arg_print_syntax( stdout, argtable, "\n" );
        fprintf( stdout, "process hash file into a header file.\n\n" );
        arg_print_glossary( stdout, argtable, "  %-25s %s\n" );
        fprintf( stdout, "\n" );

        result = 0;
    }
    else if ( gOption.version->count > 0 )   /* ditto for '--version' */
    {
        fprintf( stdout, "%s, version %s\n", global.executableName, "(to do)" );
    }
    else if ( nerrors > 0 )    /* If the parser returned any errors then display them and exit */
    {
        /* Display the error details contained in the arg_end struct.*/
        arg_print_errors( stdout, gOption.end, global.executableName );
        fprintf( stdout, "Try '%s --help' for more information.\n", global.executableName );
        result = 1;
    }
    else
    {
        global.outputFile = NULL;

#if 0
        const char * extension = ".h";
        if ( gOption.extn->count != 0 )
        {
            extension = *gOption.extn->sval;
        }
#endif
        result = 0;
        for ( int i = 0; i < gOption.file->count && result == 0; i++ )
        {
#if 0
            char output[FILENAME_MAX];
            strncpy( output, gOption.file->filename[i], sizeof( output ));


            char * p = strrchr( output, '.' );
            if ( p != NULL)
            {
                strncpy( p, extension, &output[sizeof( output ) - 1] - p );
            }

            global.outputFile = fopen( output, "w" );
            if ( global.outputFile == NULL)
            {
                fprintf( stderr, "### unable to open \'%s\' (%d: %s)\n",
                         output, errno, strerror(errno));
                result = errno;
            }
#endif
            if ( result == 0 )
            {
                result = processFile( gOption.file->filename[i] );
            }
#if 0
            fclose( global.outputFile );
#endif
        }
    }

    /* release each non-null entry in argtable[] */
    arg_freetable( argtable, sizeof( argtable ) / sizeof( argtable[0] ));

    return result;
}
