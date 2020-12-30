/**
   Copyright &copy; Paul Chambers, 2020.

   @ToDo Switch to UTF-8 string handling, rather than relying on ASCII backwards-compatibility
*/

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

#include "argtable3.h"
#include "buffer.h"

#include "keywords.h"
#include "name.h"

const char * gExecutableName;
FILE *       gOutputFile;
int          gDebugLevel = 0;

typedef unsigned long tHash;

typedef enum {
    rUnknown = 0,
    rSD,
    rHD,
    rFHD,
    rMixedHD,
    rUHD,
    rMax
} tResolution;

const char * resolutionAsString[] = {
    [rUnknown]  = "Unknown",
    [rSD]       = "SD",
    [rHD]       = "HD",
    [rFHD]      = "FHD",
    [rMixedHD]  = "Mixed HD",
    [rUHD]      = "UHD"
};

typedef enum {
    genreUnknown = 0,
    genreEntertainment,
    genreMovies,
    genreNews,
    genreDocumentary,
    genreMusic,
    genreKids,
    genreSports,
    genreReligious,
    genreMax
} tGenre;

const char * genreAsString[] = {
    [genreUnknown]       = "Unknown",
    [genreEntertainment] = "Entertainment",
    [genreMovies]        = "Movies",
    [genreNews]          = "News",
    [genreDocumentary]   = "Documentary",
    [genreMusic]         = "Music",
    [genreKids]          = "Kids",
    [genreSports]        = "Sports",
    [genreReligious]     = "Religious"
};



typedef enum {
    countryUnknown = 0,
    countryMultiple,
    countryAfghanistan,
    countryAfrica,
    countryAlbania,
    countryArab,
    countryArgentina,
    countryArmenia,
    countryAustralia,
    countryBangla,
    countryBelgium,
    countryBrazil,
    countryBulgaria,
    countryCanada,
    countryCaribbean,
    countryChina,
    countryCzech,
    countryExYu,
    countryFrance,
    countryGerman,
    countryGreece,
    countryHungary,
    countryIndia,
    countryIran,
    countryIreland,
    countryItaly,
    countryJapan,
    countryKannada,
    countryLatin,
    countryMalayalam,
    countryMarathi,
    countryNetherlands,
    countryNorway,
    countryPakistan,
    countryPhilippines,
    countryPoland,
    countryPortugal,
    countryRomania,
    countryRussia,
    countrySinhala,
    countrySpain,
    countrySweden,
    countrySwitzerland,
    countryTamil,
    countryTelugu,
    countryThailand,
    countryTurkey,
    countryUK,
    countryUSA,
    countryVietnam,
    countryMax
} tCountry;

const char * countryAsString[] = {
    [countryUnknown]     = "Unknown",
    [countryMultiple]    = "Multiple",
    [countryAfghanistan] = "Afghanistan",
    [countryAfrica]      = "Africa",
    [countryAlbania]     = "Albania",
    [countryArab]        = "Arab",
    [countryArgentina]   = "Argentina",
    [countryArmenia]     = "Armenia",
    [countryAustralia]   = "Australia",
    [countryBangla]      = "Bangla",
    [countryBelgium]     = "Belgium",
    [countryBrazil]      = "Brazil",
    [countryBulgaria]    = "Bulgaria",
    [countryCanada]      = "Canada",
    [countryCaribbean]   = "Caribbean",
    [countryChina]       = "China",
    [countryCzech]       = "Czech",
    [countryExYu]        = "ExYu",
    [countryFrance]      = "France",
    [countryGerman]      = "German",
    [countryGreece]      = "Greece",
    [countryHungary]     = "Hungary",
    [countryIndia]       = "India",
    [countryIran]        = "Iran",
    [countryIreland]     = "Ireland",
    [countryItaly]       = "Italy",
    [countryJapan]       = "Japan",
    [countryKannada]     = "Kannada",
    [countryLatin]       = "Latin",
    [countryMalayalam]   = "Malayalam",
    [countryMarathi]     = "Marathi",
    [countryNetherlands] = "Netherlands",
    [countryNorway]      = "Norway",
    [countryPakistan]    = "Pakistan",
    [countryPhilippines] = "Philippines",
    [countryPoland]      = "Poland",
    [countryPortugal]    = "Portugal",
    [countryRomania]     = "Romania",
    [countryRussia]      = "Russia",
    [countrySinhala]     = "Sinhala",
    [countrySpain]       = "Spain",
    [countrySweden]      = "Sweden",
    [countrySwitzerland] = "Switzerland",
    [countryTamil]       = "Tamil",
    [countryTelugu]      = "Telugu",
    [countryThailand]    = "Thailand",
    [countryTurkey]      = "Turkey",
    [countryUK]          = "UK",
    [countryUSA]         = "USA",
    [countryVietnam]     = "Vietnam"
};


typedef enum
{
    languageUnknown = 0,
    languageEnglish,
    languageFrench,
    languageGerman,
    languagePortugese,
    languageGreek,
    languageJapanese,
    languageChinese,
    languageSpanish,
    languageSwedish,
    languageMax
} tLanguage;

const char * languageAsString[] =
{
    [languageUnknown]   = "Unknown",
    [languageEnglish]   = "English",
    [languageFrench]    = "French",
    [languageGerman]    = "German",
    [languagePortugese] = "Portugese",
    [languageGreek]     = "Greek",
    [languageJapanese]  = "Japanese",
    [languageChinese]   = "Chinese",
    [languageSpanish]   = "Spanish",
    [languageSwedish]   = "Swedish"
};

tLanguage countryToLanguage[] =
{
    [countryUnknown]     = languageUnknown,
    [countryMultiple]    = languageUnknown,
    [countryAfghanistan] = languageUnknown,
    [countryAfrica]      = languageUnknown,
    [countryAlbania]     = languageUnknown,
    [countryArab]        = languageUnknown,
    [countryArgentina]   = languageUnknown,
    [countryArmenia]     = languageUnknown,
    [countryAustralia]   = languageEnglish,
    [countryBangla]      = languageUnknown,
    [countryBelgium]     = languageUnknown,
    [countryBrazil]      = languageUnknown,
    [countryBulgaria]    = languageUnknown,
    [countryCanada]      = languageEnglish,
    [countryCaribbean]   = languageUnknown,
    [countryChina]       = languageChinese,
    [countryCzech]       = languageUnknown,
    [countryExYu]        = languageUnknown,
    [countryFrance]      = languageFrench,
    [countryGerman]      = languageGerman,
    [countryGreece]      = languageGreek,
    [countryHungary]     = languageUnknown,
    [countryIndia]       = languageUnknown,
    [countryIran]        = languageUnknown,
    [countryIreland]     = languageEnglish,
    [countryItaly]       = languageUnknown,
    [countryJapan]       = languageJapanese,
    [countryKannada]     = languageUnknown,
    [countryLatin]       = languageSpanish,
    [countryMalayalam]   = languageUnknown,
    [countryMarathi]     = languageUnknown,
    [countryNetherlands] = languageUnknown,
    [countryNorway]      = languageUnknown,
    [countryPakistan]    = languageUnknown,
    [countryPhilippines] = languageUnknown,
    [countryPoland]      = languageUnknown,
    [countryPortugal]    = languagePortugese,
    [countryRomania]     = languageUnknown,
    [countryRussia]      = languageUnknown,
    [countrySinhala]     = languageUnknown,
    [countrySpain]       = languageSpanish,
    [countrySweden]      = languageSwedish,
    [countrySwitzerland] = languageUnknown,
    [countryTamil]       = languageUnknown,
    [countryTelugu]      = languageUnknown,
    [countryThailand]    = languageUnknown,
    [countryTurkey]      = languageUnknown,
    [countryUK]          = languageEnglish,
    [countryUSA]         = languageEnglish,
    [countryVietnam]     = languageUnknown
};

typedef enum
{
    affiliateUnknown = 0,
    affiliateABC,
    affiliateBBC,
    affiliateCBC,
    affiliateCBS,
    affiliateCTV,
    affiliateCW,
    affiliateFox,
    affiliateITV,
    affiliateNBC,
    affiliatePBS,
    affiliateMax
} tAffiliate;

const char * affiliateAsString[] =
{
   [affiliateUnknown] = "Unknown",
   [affiliateABC]     = "ABC",
   [affiliateBBC]     = "BBC",
   [affiliateCBC]     = "CBC",
   [affiliateCBS]     = "CBS",
   [affiliateCTV]     = "CTV",
   [affiliateCW]      = "CW",
   [affiliateFox]     = "Fox",
   [affiliateITV]     = "ITV",
   [affiliateNBC]     = "NBC",
   [affiliatePBS]     = "PBS"
};

tCountry affiliateToCountry[] =
{
     [affiliateUnknown] = countryUnknown,
     [affiliateABC]     = countryUSA,
     [affiliateBBC]     = countryUK,
     [affiliateCBC]     = countryCanada,
     [affiliateCBS]     = countryUSA,
     [affiliateCTV]     = countryCanada,
     [affiliateCW]      = countryUSA,
     [affiliateFox]     = countryUSA,
     [affiliateITV]     = countryUK,
     [affiliateNBC]     = countryUSA,
     [affiliatePBS]     = countryUSA
};


typedef struct {
    const char *  name;
    tHash         hash;
} tLineup;



typedef struct sGroup {
    struct sGroup * next;
    const char *    name;
    tHash           hash;
    tCountry        country;
    tLanguage       language;
    tGenre          genre;
    tAffiliate      affiliate;
    tResolution     resolution;
    bool            isVIP;
} tGroup;

typedef struct sStream {
    struct sStream *  next;
    const char     *  url;
    tHash             hash;
    struct sChannel * channel;
    tResolution       resolution;
    bool              isVIP;
} tStream;

typedef struct sChannel {
    struct sChannel * next;

    const char *      name;
    tHash             hash;
    tCountry          country;
    tLanguage         language;
    tGenre            genre;
    tAffiliate        affiliate;

    tStream *         stream;
    tLineup *         lineup;
    tGroup *          group;
} tChannel;

tGroup *   groupHead   = NULL;
tChannel * channelHead = NULL;
tStream *  streamHead  = NULL;


tStream * newStream( void )
{
    return (tStream *) calloc( 1, sizeof( tStream ));
}

void freeStream( tStream * stream )
{
    free( (void *)stream );
}

tChannel * newChannel(void)
{
    return (tChannel *) calloc(1,sizeof( tChannel ));
}

tChannel * freeChannel( tChannel * channel )
{
    if (channel != NULL)
    {
        tStream * stream = channel->stream;
        channel->stream = NULL;
        while ( stream != NULL)
        {
            tStream * next = stream->next;
            freeStream( stream );
            stream = next;
        }

        if ( channel->lineup != NULL)
        {
            free( channel->lineup );
            channel->lineup = NULL;
        }

        free( channel );
    }
    return NULL;
}

tGroup * newGroup( void )
{
    return (tGroup *) calloc( 1, sizeof( tGroup ));
}

void freeGroup( tGroup * group )
{
    if (group != NULL)
    {
        free((void *) group->name);
        free((void *) group );
    }
}

void dumpStream( tStream * stream )
{
    fprintf( stderr, "      rez: %s, isvip %d, url: %s\n",
             resolutionAsString[stream->resolution],
             stream->isVIP,
             stream->url );
}

void dumpChannel( tChannel * channel )
{
    fprintf( stderr,
             "    channel: %s (0x%08lx), genre: %s, affiliate: %s, country: %s, language: %s\n",
             channel->name,
             channel->hash,
             genreAsString[channel->genre],
             affiliateAsString[channel->affiliate],
             countryAsString[channel->country],
             languageAsString[channel->language] );
}

void dumpGroup( tGroup * group )
{
    fprintf( stderr, "    group: %s (0x%08lx), genre: %s, affiliate: %s, country: %s, language: %s, rez: %s, vip: %d\n",
             group->name,
             group->hash,
             genreAsString[group->genre],
             affiliateAsString[group->affiliate],
             countryAsString[group->country],
             languageAsString[group->language],
             resolutionAsString[group->resolution],
             group->isVIP );
}

unsigned int calcNameHash( const char * string )
{
    tHash hash = 0;
    const unsigned char * p = (const unsigned char *)string;
    unsigned int c;

    while ( (c = *p++) != '\0' )
    {
        c = gNameCharMap[c];
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

/**
 * @brief
 * @param hash
 * @param vip
 * @param resolution
 * @return
 */
bool checkHashResolutionVIP( tHash hash, bool * vip, tResolution * resolution )
{
    bool result = false;

    switch ( hash )
    {
    case kNameVIP:
        *vip = true;
        break;

    case kNameSD:
        *resolution = rSD;
        break;

    case kNameHD:
        *resolution = rHD;
        break;

    case kNameFHD:
        *resolution = rFHD;
        break;

    case kNameHDMix:
    case kNameFHDMix:
        *resolution = rMixedHD;
        break;

    default:
        result = true;
        break;
    }
    return result;
}

/**
 * @brief
 * @param hash
 * @param country
 * @return
 */
bool checkHashCountry( tHash hash, tCountry * country, tLanguage * language )
{
    bool result = false;

    tCountry previous = *country;

    switch ( hash )
    {
    case kNameAfghanistan:
        *country = countryAfghanistan;
        break;
    case kNameAfrica:
        *country = countryAfrica;
        break;
    case kNameAlbania:
        *country = countryAlbania;
        break;
    case kNameArab:
        *country = countryArab;
        break;
    case kNameArgentina:
        *country = countryArgentina;
        break;
    case kNameArmenia:
        *country = countryArmenia;
        break;
    case kNameAustralia:
    case kNameAustraliaNZ:
        *country  = countryAustralia;
        break;
    case kNameBangla:
        *country = countryBangla;
        break;
    case kNameBelgium:
        *country = countryBelgium;
        break;
    case kNameBrazil:
        *country  = countryBrazil;
        break;
    case kNameBulgaria:
        *country = countryBulgaria;
        break;
    case kNameCanada:
        *country  = countryCanada;
        break;
    case kNameCaribbean:
        *country = countryCaribbean;
        break;
    case kNameChina:
        *country  = countryChina;
        break;
    case kNameCzech:
        *country = countryCzech;
        break;
    case kNameExYu:
        *country = countryExYu;
        break;
    case kNameFrance:
        *country = countryFrance;
        break;
    case kNameGerman:
        *country = countryGerman;
        break;
    case kNameGreece:
    case kNameGreek:
        *country = countryGreece;
        break;
    case kNameHungary:
        *country = countryHungary;
        break;
    case kNameIndia:
        *country = countryIndia;
        break;
    case kNameIran:
        *country = countryIran;
        break;
    case kNameIreland:
    case kNameIrish:
        *country  = countryIreland;
        break;
    case kNameItaly:
        *country = countryItaly;
        break;
    case kNameJapan:
        *country  = countryJapan;
        break;
    case kNameKannada:
        *country = countryKannada;
        break;
    case kNameLatin:
        *country  = countryLatin;
        break;
    case kNameMalayalam:
        *country = countryMalayalam;
        break;
    case kNameMarathi:
        *country = countryMarathi;
        break;
    case kNameNetherlands:
        *country = countryNetherlands;
        break;
    case kNameNorway:
        *country = countryNorway;
        break;
    case kNamePakistan:
        *country = countryPakistan;
        break;
    case kNamePhilippines:
        *country = countryPhilippines;
        break;
    case kNamePolish:
        *country = countryPoland;
        break;
    case kNamePortugal:
        *country  = countryPortugal;
        break;
    case kNameRomania:
        *country = countryRomania;
        break;
    case kNameRussia:
        *country = countryRussia;
        break;
    case kNameSinhala:
        *country = countrySinhala;
        break;
    case kNameSpain:
    case kNameSpanish:
        *country  = countrySpain;
        break;
    case kNameSweden:
        *country  = countrySweden;
        break;
    case kNameSwitzerland:
        *country = countrySwitzerland;
        break;
    case kNameTamil:
        *country = countryTamil;
        break;
    case kNameTelugu:
        *country = countryTelugu;
        break;
    case kNameThailand:
        *country = countryThailand;
        break;
    case kNameTurkey:
    case kNameTurkish:
        *country = countryTurkey;
        break;
    case kNameUK:
        *country  = countryUK;
        break;
    case kNameUS:
    case kNameUSA:
        *country  = countryUSA;
        break;
    case kNameVietnam:
        *country = countryVietnam;
        break;

    default:
        result = true;
        break;
    }
    if (result == false)
    {
        *language = countryToLanguage[*country];

        if ( previous != countryUnknown
          || previous == countryMultiple )
        {
            *country = countryMultiple;
        }
    }
    return result;
}

/**
 * @brief
 * @param hash
 * @param genre
 * @return
 */
bool checkHashGenre( tHash hash, tGenre * genre )
{
    bool result = false;

    switch ( hash )
    {
    case kNameEntertainment:
        *genre = genreEntertainment;
        break;

    case kNameNews:
        *genre = genreNews;
        break;

    case kNameMusic:
    case kNameRadio:
        *genre = genreMusic;
        break;

    case kNameDocumentary:
    case kNameDocumentaries:
        *genre = genreDocumentary;
        break;

    case kNameSport:
    case kNameSports:
    case kNameFootball:
    case kNameSoccer:
    case kNameRugby:
    case kNameLeague:
    case kNameFormula:
    case kNameMotorsports:
    case kNameRacing:
    case kNameChampionship:
    case kNameGolf:
    case kNameESPN:
        *genre = genreSports;
        break;

    case kNameNFL:
    case kNameNHL:
    case kNameNBA:
    case kNameMLB:
        *genre = genreSports;
        break;

    case kNameMovie:
    case kNameMovies:
        *genre = genreMovies;
        break;

    case kNameChildrens:
    case kNameKids:
        *genre = genreKids;
        break;

    case kNameReligious:
        *genre = genreReligious;
        break;

    default:
        result = true;
        break;
    }
    return result;
}

/**
 * @brief
 * @param hash
 * @param affiliate
 * @return
 */
bool checkHashAffiliate( tHash hash, tAffiliate * affiliate )
{
    bool result = false;

    switch ( hash )
    {
    case kNameBBC:
        *affiliate = affiliateBBC;
        break;

    case kNameITV:
        *affiliate = affiliateITV;
        break;

    case kNameABC:
        *affiliate = affiliateABC;
        break;

    case kNameCBC:
        *affiliate = affiliateCBC;
        break;

    case kNameCBS:
        *affiliate = affiliateCBS;
        break;

    case kNameCW:
        *affiliate = affiliateCW;
        break;

    case kNameFox:
        *affiliate = affiliateFox;
        break;

    case kNameNBC:
        *affiliate = affiliateNBC;
        break;

    case kNamePBS:
        *affiliate = affiliatePBS;
        break;

    default:
        result = true;
        break;
    }
    return result;
}

/**
 * @brief
 * @param stream
 * @param name
 * @return
 */
tChannel * processChannelName( tStream * stream, const char * name )
{
    tChannel *     channel;
    unsigned int   mappedC;
    const char *   p = name;
    const char *   sp;
    char *         dp;
    int            dl;
    tHash          hash;
    char           temp[64];

    channel = newChannel();

    /* first extract any attributes embedded in the channel name,
     * tags like 'VIP', 'UK', 'HD', etc. See name.hash */

    hash = 0;

    p = name;
    sp = p;

    temp[0] = '\0';
    dp = temp;
    dl = sizeof( temp ) - 1;

    do
    {
        mappedC = getNameWord( *p++ );
        if ( mappedC != kNameSeparator && mappedC != '\0' )
        {
            hash = fNameHashChar( hash, mappedC );
        }
        else {
            if ( hash != 0 )
            {
                if ( checkHashResolutionVIP( hash, &stream->isVIP,
                                             &stream->resolution ))
                {
                    if ( checkHashCountry( hash, &channel->country, &channel->language ))
                    {
                        if ( checkHashGenre( hash, &channel->genre ))
                        {
                            checkHashAffiliate( hash, &channel->affiliate );
                        }
                    }

                    while ( dl > 0 && p > sp )
                    {
                        *dp++ = *sp++;
                        dl--;
                    }
                    *dp = '\0';
                }
                hash = 0;
            }
            sp = p;
        }
    } while ( mappedC != '\0' );

    trimTrailingWhitespace( temp );

    /* Let's see if we already have a matching channel */

    channel->name = strdup( temp );
    channel->hash = calcNameHash( temp );

    tChannel * chan = channelHead;
    while ( chan != NULL )
    {
        if ( channel->hash == chan->hash
          && channel->country == chan->country )
        {
            /* channel already exists, so discard the local one */
            freeChannel( channel );
            /* and switch to the existing one */
            channel = chan;
            break;
        }
        chan = chan->next;
    }
    if (chan == NULL)
    {
        /* didn't find it, so add new channel to the chain */
        channel->next = channelHead;
        channelHead   = channel;
    }

    /* point to the channel from the stream */
    stream->channel = channel;

    return channel;
}

tGroup * processGroupName( const char * name )
{
    tGroup *       group;
    const char *   p;
    const char *   sp;
    char *         dp;
    size_t         dl;
    unsigned int   mappedC;
    unsigned long  hash;
    char temp[64];

    p  = name;
    sp = p;

    temp[0] = '\0';
    dp = temp;
    dl = sizeof(temp) - 1;

    hash = 0;

    group = newGroup();

    do {
        mappedC = getNameWord( *p++ );
        if ( mappedC != kNameSeparator && mappedC != '\0' )
        {
            hash = fNameHashChar( hash, mappedC );
        }
        else {
            if ( hash != 0 )
            {
                if ( checkHashResolutionVIP( hash, &group->isVIP,
                                             &group->resolution ))
                {
                    if ( checkHashCountry( hash, &group->country, &group->language ))
                    {
                        if ( checkHashGenre( hash, &group->genre ))
                        {
                            checkHashAffiliate( hash, &group->affiliate );
                        }
                    }

                    while ( dl > 0 && sp < p )
                    {
                        *dp++ = *sp++;
                        dl--;
                    }
                    *dp = '\0';
                }
                hash = 0;
            }
            sp = p;
        }
    } while ( mappedC != '\0' );

    group->name = strdup( temp );
    group->hash = calcNameHash( temp );

    tGroup * grp = groupHead;
    while ( grp != NULL)
    {
        if ( group->hash == grp->hash
          && group->country == grp->country )
        {
            /* channel already exists, so discard the local one */
            freeGroup( group );
            /* and switch to the existing one */
            group = grp;
            fprintf( stderr, "existing group \"%s\" (0x%08lx)\n",
                     group->name, group->hash );
            break;
        }
        grp = grp->next;
    }
    if ( grp == NULL)
    {
        /* didn't find it, so add new channel to the chain */
        group->next = groupHead;
        groupHead = group;
        fprintf( stderr, "new group \"%s\" (0x%08lx)\n",
                 group->name, group->hash);
    }

    return group;
}

void ProcessEntry( tGroup * group, tChannel * channel, tStream * stream )
{
    fprintf( stderr, ">>> ProcessEntry\n" );

    /* inherit group attributes as applicable */

    /* inherit country from group if channel country is unknown */
    if (channel->country == countryUnknown
     && group->country != countryUnknown )
    {
        channel->country = group->country;
    }

    if ( channel->language == languageUnknown
      && group->language != languageUnknown )
    {
        channel->language = group->language;
    }

    if (channel->genre == genreUnknown
     && group->genre != genreUnknown)
    {
        channel->genre = group->genre;
    }

    if ( channel->affiliate == affiliateUnknown
      && group->affiliate != affiliateUnknown )
    {
        channel->affiliate = group->affiliate;
    }

    /* inherit resolution from group if channel resolution is unknown */
    if ( stream->resolution == rUnknown
      && group->resolution != rUnknown )
    {
        stream->resolution = group->resolution;
    }

    /* inherit VIP status from group if not already VIP */
    if ( !stream->isVIP && group->isVIP)
    {
        stream->isVIP = group->isVIP;
    }

    dumpGroup( group );
    dumpChannel( channel );
    dumpStream( stream );
}

/* process an entire M3U file */
void processM3U( tBuffer * buffer )
{
    unsigned long hash = 0;
    unsigned long assignmentHash = 0;

    tGroup *   group   = NULL;
    tChannel * channel = NULL;
    tStream *  stream  = NULL;

    while ( bufferGetRemaining( buffer ) > 0 )
    {
        unsigned short w = getKeywordWord( bufferGetChar( buffer ));
        switch ( w )
        {
        case kKeywordEOL: /* check hash for a known keyword */
            switch (hash)
            {
            case kKeywordEXTM3U:
                fprintf(stderr, "[File Start]\n" );
                break;

            default:
                fprintf( stderr, "### unknown line\n" );
                break;
            }
            hash = 0;
            break;

        case kKeywordSeparator: /* check hash for a known keyword */
            switch ( hash )
            {
            case kKeywordEXTINF:
                printf( "[Entry Start]\n" );
                stream  = newStream();
                group   = newGroup();
                break;
            }
            hash = 0;
            break;

        case kKeywordAssign:
            if ( hash != 0 )
            {
                assignmentHash = hash;
                hash           = 0;
            }
            break;

        case kKeywordQuote:
            {
                const char * str = bufferGetQuotedString( buffer );
                if ( str != NULL)
                {
                    switch ( assignmentHash )
                    {
                    case kKeywordID:
                        printf( "       ID: \"%s\"\n", str );
                        break;

                    case kKeywordName:
                        {
                            printf( "     Name: \"%s\"\n", str );

                            channel = processChannelName( stream, str );
                        }
                        break;

                    case kKeywordLogo:
                        // printf( "     Logo: \"%s\"\n", str );
                        break;

                    case kKeywordGroup:
                        {
                            printf( "    Group: \"%s\"\n", str );

                            group = processGroupName( str );
                        }
                        break;
                    }
                    assignmentHash = 0;
                }
                hash = 0;
            }
            break;

        case kKeywordComma:
            {
                /* from here to EOL is also the name */
                const char * name = bufferGetStringToEOL( buffer );
                printf( " trailing: \"%s\"\n", name );

                /* the entire next line is the URL */
                const char * url = bufferGetStringToEOL( buffer );
                printf( "      url: \"%s\"\n", url );

                stream->url  = url;
                stream->hash = calcNameHash( url );

                /* We've finished parsing an Entry, now incorporate it */
                ProcessEntry( group, channel, stream );

                group   = NULL;
                channel = NULL;

                hash = 0;
            }
            break;

        default:
            hash = fKeywordHashChar( hash, w );
            break;
        }
    } /* getBuffRemaining */

    fprintf(stderr, "[Groups]\n");

    group = groupHead;
    while (group != NULL)
    {
        dumpGroup( group );
        group = group->next;
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

        if ( map == MAP_FAILED || map == NULL )
        {
            result = -errno;
        }
        else
        {
            tBuffer * buffer = bufferNew( map, length );
            processM3U( buffer );
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
