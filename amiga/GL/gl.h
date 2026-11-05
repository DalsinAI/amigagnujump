/* No OpenGL on AmigaOS/acsdl12. GNUjump picks its renderer at run time
 * (gblOps.useGL), so its GL code is compiled but never reached: the Amiga
 * build forces the software renderer (setWindow) and links amiga/gl2d_none.c
 * in place of SDL_2dgl.c. These are the names that code mentions. */
#ifndef GNUJUMP_AMIGA_GL_H
#define GNUJUMP_AMIGA_GL_H

typedef int GLint;
typedef unsigned int GLuint;
typedef unsigned int GLenum;
typedef float GLfloat;
typedef double GLdouble;
typedef unsigned char GLubyte;

#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_MAX_TEXTURE_SIZE 0x0D33
#define GL_RENDERER 0x1F01
#define GL_DEPTH_TEST 0x0B71
#define GL_CULL_FACE 0x0B44
#define GL_PROJECTION 0x1701
#define GL_MODELVIEW 0x1700
#define GL_COLOR_BUFFER_BIT 0x00004000

static inline const GLubyte *glGetString(GLenum n) { (void)n; return (const GLubyte *)""; }
static inline void glGetIntegerv(GLenum n, GLint *v) { (void)n; *v = 0; }
static inline void glDisable(GLenum n) { (void)n; }
static inline void glViewport(GLint x, GLint y, GLint w, GLint h) { (void)x; (void)y; (void)w; (void)h; }
static inline void glMatrixMode(GLenum n) { (void)n; }
static inline void glLoadIdentity(void) {}
static inline void glOrtho(GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f)
{ (void)l; (void)r; (void)b; (void)t; (void)n; (void)f; }
static inline void glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { (void)r; (void)g; (void)b; (void)a; }
static inline void glClear(GLuint m) { (void)m; }
static inline void glFinish(void) {}

/* SDL 1.2's GL attribute calls, which acsdl12 does not have. */
#define SDL_GL_RED_SIZE 0
#define SDL_GL_GREEN_SIZE 1
#define SDL_GL_BLUE_SIZE 2
#define SDL_GL_DEPTH_SIZE 5
#define SDL_GL_DOUBLEBUFFER 6
static inline int SDL_GL_SetAttribute(int attr, int value) { (void)attr; (void)value; return -1; }
static inline void SDL_GL_SwapBuffers(void) {}

#endif
