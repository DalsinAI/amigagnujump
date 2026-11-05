/* GNUjump on AmigaOS / acsdl12: the screen palette of a skin, and alpha
 * blending on an 8-bit screen.
 *
 * GNUjump's software renderer was written for 16 and 32-bit screens. Its
 * pictures are drawn opaque or colour-keyed, which an 8-bit screen does as
 * well, once every picture is in the screen's palette: amiga/tools/skin8.py
 * gives each skin one palette and writes its pictures in it, and
 * amiga_skin_palette() puts that palette on the screen when the skin loads.
 * What does blend is drawSquareAlpha (the menu highlight, the message boxes)
 * and drawLine (the players' trails): each pixel read back, mixed with a
 * colour and matched to the screen's palette. Here the match is a table
 * per colour and alpha, built from an inverse colour map that fills in as
 * it is used, so a blended pixel costs one table look-up.
 *
 * The AmiCradle Team. GPL 3, like GNUjump.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "amiga.h"

/* ------------------------------------------------------------ palette */

void amiga_skin_palette(SDL_Surface *screen, const char *skindir)
{
    char path[512];
    SDL_Surface *pal;
    SDL_Color colors[256];
    int i;

    if (!screen || screen->format->BitsPerPixel != 8)
        return;
    snprintf(path, sizeof(path), "%s/palette.png", skindir);
    pal = IMG_Load(path);
    if (pal && pal->format->palette) {
        memcpy(colors, pal->format->palette->colors, sizeof(colors));
    } else {
        /* A skin of the player's own, not run through skin8.py: 3-3-2, as
         * acsdl12's PNG loader gives its larger pictures. */
        fprintf(stderr, "No %s: a 3-3-2 palette for this skin.\n", path);
        for (i = 0; i < 256; i++) {
            int r = (i >> 5) & 7, g = (i >> 2) & 7, b = i & 3;
            colors[i].r = (Uint8)(r << 5 | r << 2 | r >> 1);
            colors[i].g = (Uint8)(g << 5 | g << 2 | g >> 1);
            colors[i].b = (Uint8)(b << 6 | b << 4 | b << 2 | b);
            colors[i].unused = 0;
        }
    }
    if (pal)
        SDL_FreeSurface(pal);
    SDL_SetColors(screen, colors, 0, 256);
}

/* ------------------------------------------------------------ blending */

#define INV_BITS 5
#define INV_SIZE (1 << (3 * INV_BITS))
#define INV_NONE 0xffff
#define MAPS 96             /* 4 trail colours x 16 steps, and the rest */

typedef struct {
    Uint32 version;
    Uint8 r, g, b, level;
    Uint8 map[256];
} BlendMap;

static Uint16 *g_inv;           /* 5-5-5 colour -> nearest pen, or INV_NONE */
static Uint32 g_inv_version;
static BlendMap g_maps[MAPS];
static int g_nmaps, g_next;

static Uint8 nearest_pen(const SDL_Color *pal, int r, int g, int b)
{
    int i, best = 0;
    long best_d = 0x7fffffffL;

    for (i = 0; i < 256; i++) {
        long dr = pal[i].r - r, dg = pal[i].g - g, db = pal[i].b - b;
        long d = dr * dr * 3 + dg * dg * 4 + db * db * 2;
        if (d < best_d) {
            best_d = d;
            best = i;
            if (!d)
                break;
        }
    }
    return (Uint8)best;
}

/* The pen nearest (r, g, b), through the inverse map. */
static Uint8 pen_for(const SDL_Color *pal, int r, int g, int b)
{
    int k = (r >> (8 - INV_BITS)) << (2 * INV_BITS) | (g >> (8 - INV_BITS)) << INV_BITS |
            (b >> (8 - INV_BITS));

    if (g_inv[k] == INV_NONE) {
        /* Match the middle of the 5-5-5 cell. */
        int h = 1 << (7 - INV_BITS);
        g_inv[k] = nearest_pen(pal, (r & ~(2 * h - 1)) | h, (g & ~(2 * h - 1)) | h,
                               (b & ~(2 * h - 1)) | h);
    }
    return (Uint8)g_inv[k];
}

const unsigned char *amiga_blend_map(SDL_Surface *dst, unsigned char r, unsigned char g,
                                     unsigned char b, unsigned char alpha)
{
    const SDL_Color *pal;
    BlendMap *m;
    Uint32 version;
    int i, level, a;

    if (!dst || !dst->format->palette)
        return NULL;
    pal = dst->format->palette->colors;
    version = dst->acsdl_palette_version;
    if (!g_inv) {
        g_inv = (Uint16 *)malloc(INV_SIZE * sizeof(Uint16));
        if (!g_inv)
            return NULL;
        g_inv_version = version + 1;
    }
    if (g_inv_version != version) {
        memset(g_inv, 0xff, INV_SIZE * sizeof(Uint16));
        g_inv_version = version;
        g_nmaps = g_next = 0;
    }
    level = alpha >> 4;         /* 16 steps */
    for (i = 0; i < g_nmaps; i++) {
        m = &g_maps[i];
        if (m->level == level && m->r == r && m->g == g && m->b == b)
            return m->map;
    }
    if (g_nmaps < MAPS)
        m = &g_maps[g_nmaps++];
    else {
        m = &g_maps[g_next];
        g_next = (g_next + 1) % MAPS;
    }
    m->version = version;
    m->r = r;
    m->g = g;
    m->b = b;
    m->level = (Uint8)level;
    a = level * 17;             /* 0..255 */
    for (i = 0; i < 256; i++) {
        int br = (r * a + pal[i].r * (255 - a)) / 255;
        int bg = (g * a + pal[i].g * (255 - a)) / 255;
        int bb = (b * a + pal[i].b * (255 - a)) / 255;
        m->map[i] = pen_for(pal, br, bg, bb);
    }
    return m->map;
}

void amiga_blend_rect(SDL_Surface *dst, unsigned char r, unsigned char g, unsigned char b,
                      unsigned char alpha, int x, int y, int w, int h)
{
    const Uint8 *map;
    Uint8 *row;
    int i, j;

    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (x + w > dst->w)
        w = dst->w - x;
    if (y + h > dst->h)
        h = dst->h - y;
    if (w <= 0 || h <= 0 || alpha < 16)
        return;
    map = amiga_blend_map(dst, r, g, b, alpha);
    if (!map)
        return;
    row = (Uint8 *)dst->pixels + y * dst->pitch + x;
    for (j = 0; j < h; j++, row += dst->pitch)
        for (i = 0; i < w; i++)
            row[i] = map[row[i]];
}
