/* GNUjump's config.h for the AmigaOS build (configure does not run here).
 * No NLS: the game is in English. The data are found through PROGDIR:
 * (amiga/amiga.h), so DATA_PREFIX is unused. */
#define PACKAGE "gnujump"
#define PACKAGE_NAME "gnujump"
#define PACKAGE_VERSION "1.0.8"
#define VERSION "1.0.8"
#define DATA_PREFIX "PROGDIR:"
#define LOCALEDIR "PROGDIR:locale"
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_UNISTD_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define STDC_HEADERS 1

#include "amiga.h"
