/* GNUjump on AmigaOS / acsdl12: where things are, and the 8-bit helpers.
 * The AmiCradle Team. GPL 3, like GNUjump. */
#ifndef GNUJUMP_AMIGA_H
#define GNUJUMP_AMIGA_H

#include <sys/stat.h>   /* the S_I* bits setup.c's mkdir() uses */

/* Everything beside the program: the skins, the sounds, and the writable
 * config, records and replays (there is no home directory to put them in). */
#define AMIGA_SKINS   "PROGDIR:skins"
#define AMIGA_SOUND   "PROGDIR:sound"
#define AMIGA_REPLAYS "PROGDIR:replays"
#define AMIGA_CFGFILE "PROGDIR:gnujump.cfg"
#define AMIGA_HSCFILE "PROGDIR:gnujump.hsc"

struct SDL_Surface;

/* Sets the screen's palette from the skin's palette.png (amiga/tools/skin8.py
 * wrote every picture of the skin in that palette), or 3-3-2 for a skin
 * without one. */
void amiga_skin_palette(struct SDL_Surface *screen, const char *skindir);

/* A colour (r, g, b) over the 8-bit screen at alpha: for each pen of the
 * screen's palette, the pen nearest the blend. Cached; alpha in 16 steps. */
const unsigned char *amiga_blend_map(struct SDL_Surface *dst, unsigned char r,
                                     unsigned char g, unsigned char b,
                                     unsigned char alpha);

/* drawSquareAlpha on an 8-bit surface, clipped on all four sides. */
void amiga_blend_rect(struct SDL_Surface *dst, unsigned char r, unsigned char g,
                      unsigned char b, unsigned char alpha, int x, int y, int w, int h);

#endif
