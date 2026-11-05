/* GNUjump keeps its 512-byte config strings on the stack, several deep
 * (getValue_str inside loadGraphics, the menus), and the C library's
 * fscanf wants its share; a Shell gives 4 KB. libnix's startup sets this
 * much up instead. The AmiCradle Team. GPL 3, like GNUjump. */
unsigned long __stack = 128 * 1024;
