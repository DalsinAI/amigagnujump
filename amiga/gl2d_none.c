/* GNUjump's OpenGL renderer (src/SDL_2dgl.c) on a machine without OpenGL:
 * the calls the software path still names, none of them ever reached, since
 * the Amiga build always renders in software (setWindow). Linked in place
 * of SDL_2dgl.c. The AmiCradle Team. GPL 3, like GNUjump. */

#include <stdlib.h>
#include "SDL_2dgl.h"

void GL2D_DrawLine(Uint8 r, Uint8 g, Uint8 b, Uint8 a, int x0, int y0, int x1, int y1)
{
    (void)r; (void)g; (void)b; (void)a; (void)x0; (void)y0; (void)x1; (void)y1;
}

void GL2D_DrawRect(Uint8 r, Uint8 g, Uint8 b, Uint8 a, int x, int y, int w, int h)
{
    (void)r; (void)g; (void)b; (void)a; (void)x; (void)y; (void)w; (void)h;
}

GL2D_SurfaceGL *GL2D_CreateSurfaceGL(SDL_Surface *surf, GLint filter)
{
    (void)filter;
    GL2D_SurfaceGL *s = calloc(1, sizeof(*s));
    if (s && surf) {
        s->w = (Uint16)surf->w;
        s->h = (Uint16)surf->h;
    }
    return s;
}

GL2D_LargeSurfaceGL *GL2D_CreateLargeSurfaceGL(SDL_Surface *surf, GLint filter)
{
    (void)filter;
    GL2D_LargeSurfaceGL *s = calloc(1, sizeof(*s));
    if (s && surf) {
        s->w = (Uint16)surf->w;
        s->h = (Uint16)surf->h;
    }
    return s;
}

void GL2D_FreeSurfaceGL(GL2D_SurfaceGL *src) { free(src); }
void GL2D_FreeLargeSurfaceGL(GL2D_LargeSurfaceGL *src) { free(src); }

void GL2D_BlitGLrot(GL2D_SurfaceGL *src, float x, float y, float angle, Uint8 alpha)
{
    (void)src; (void)x; (void)y; (void)angle; (void)alpha;
}

void GL2D_BlitSurfaceGL(GL2D_SurfaceGL *src, SDL_Rect *src_r, SDL_Rect *dest_r, Uint8 alpha)
{
    (void)src; (void)src_r; (void)dest_r; (void)alpha;
}

void GL2D_BlitLargeSurfaceGL(GL2D_LargeSurfaceGL *src, SDL_Rect *src_r, SDL_Rect *dest_r,
                             Uint8 alpha)
{
    (void)src; (void)src_r; (void)dest_r; (void)alpha;
}
