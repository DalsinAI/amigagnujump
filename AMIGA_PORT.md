# GNUjump for AmigaOS 3.x

- Upstream: `https://savannah.gnu.org/projects/gnujump/`
- Imported: the 1.0.8 release, `gnujump-1.0.8.tar.gz` from ftp.gnu.org
  (Savannah's git did not answer), unchanged in the first commit
- Licence: GPL 3 (`COPYING`)

## Target

AmigaOS 3.x / AmiCradle, Motorola 68040 + FPU, on acsdl12 (our SDL 1.2 on
ACGame and AHI). GNUjump's software renderer on an 8-bit screen; its
OpenGL renderer is not built (acsdl12 has no OpenGL).

## Build

    make -f amiga/Makefile.amigaos \
        STOVE=/path/to/openamigagcc-16.2.0-open4-x86_64-linux \
        NDK=/path/to/gfopensdk/OpenNDK/Include_H \
        ACSDL=/path/to/amicradle-guest/gameports/acsdl12 \
        ACSDLLIB=/path/to/acsdl12/build ACGAME=/path/to/acgame \
        ACGAMELIB=/path/to/acgame/build-amiga

acsdl12 is built with its own `Makefile.amigaos`. This gives
`build/amigaos/GNUjump/` with `gnujump`, `skins/`, `sound/`, `replays/`,
this ReadMe, `COPYING`, an icon for each of them and for every drawer in
`skins/`, and `GNUjump.info` beside the drawer. Run it from that drawer:
everything is found through `PROGDIR:`. The icons are made from the
default skin's hero with AmiCradle's `port_icons.py` from
`amiga/icons/icons.json`.

## Installing

Unpack the archive anywhere (on AmiCradle: `SYS:Utilities/OpenCompress x
gnujump-...lha RAM:`), then double-click `Install-GNUjump` beside the
GNUjump drawer: it asks where to put the game (`Work:Games` by default)
and keeps the replays, options and records of an earlier install. The
drawer's `Libs` holds the acgame.library the game was built against, and
the game opens that copy first. `package/build-amiga.sh` makes the
archive (STOVE and OUT set; it fetches and builds acgame, acsdl12 and zlib
at pinned commits).

## Playing

640x480 in 256 colours (the xjump skin 540x592), RTG or AGA
(`ENV:ACGame/Display`). Player 1: cursor keys, Up jumps; players 2-4: `A`
`D` `W`, `J` `L` `I`, keypad 4 6 8. `P` pauses, Esc leaves. The menus take
the keys and the mouse. Five skins (Options > Choose Theme): default, auri,
sdljump, wincrash, xjump.

Written beside the program: `gnujump.cfg` (options), `gnujump.hsc`
(records) and replays in `replays/`, so the drawer must be on a writable
disk for them to be kept.

## Graphics in 256 colours

GNUjump's pictures are RGBA PNGs drawn on 16 or 32-bit screens; the menu
and game backgrounds have tens of thousands of colours. acsdl12 has 8-bit
screens only, and its PNG loader cuts a picture of more than 256 colours to
3-3-2. What the game blends is little: its sprites and fonts are drawn
opaque or colour-keyed, and true alpha is used only by `drawSquareAlpha`
(the menu highlight, message boxes) and `drawLine` (the players' trails).

- `amiga/tools/skin8.py` (host, Pillow and numpy) gives each skin one
  palette made from all its pictures and writes them as paletted PNGs in
  it: index 0 black and the colour key (pixels under half alpha), 1
  magenta for SFont's markers, 2-255 the skin's colours, backgrounds
  dithered. Its output, `amiga/skins`, is committed; the Amiga build copies
  it. `palette.png` in each skin holds the palette.
- `amiga/amiga.c`: `amiga_skin_palette` puts the skin's palette on the
  screen when the skin loads (`loadGraphics`), so every blit is a copy;
  a skin without `palette.png` gets 3-3-2. `amiga_blend_map` and
  `amiga_blend_rect` blend on the 8-bit screen through a table per colour
  and alpha (16 steps), built from an inverse colour map that fills in as
  it is used.
- Lost: anti-aliased edges (half-transparent pixels become solid or
  clear), the arrows' fade in the menus (drawn solid or not at all), and
  sprite smoothing when rotating (8-bit rotozoom is nearest-pixel). The
  blur effect was OpenGL-only upstream.

## Sound

The sounds are Ogg Vorbis, which acsdl12 does not read: `amiga/sound` holds
them as WAVs (mono, 16-bit, 22050 Hz, made with GStreamer's `vorbisdec`),
and the Makefile writes `sound/config.sounds` naming them. The music,
`silence.ogg`, is silence and is left out.

## Changes for the Amiga

Under `#ifdef __amigaos__` unless said:

- `src/main.c`, `src/setup.c`: config and records in `PROGDIR:`, skins in
  `PROGDIR:skins`, replays in `PROGDIR:replays` (made if missing), no
  gettext; `setWindow` always opens an 8-bit software screen, whatever the
  config's OpenGL and colour-depth options say; `loadGraphics` sets the
  skin's palette.
- `src/gnujump.h`: the default sounds in `PROGDIR:sound`.
- `src/surface.c`: `drawLine` and `drawSquareAlpha` blend through
  `amiga/amiga.c` on an 8-bit surface.
- `src/replay.c`: see byte order. Also `endReplay`'s write past the end of
  `heros[]` (upstream bug, multiplayer replays) is left out, and the replay
  comment reader no longer runs past its 512 bytes or loops at end of file.
- `src/tools.c`: `snprintf` for a directory entry's path (a long name
  overran the 256-byte buffer).
- `amiga/config.h` in place of configure's; `amiga/GL/gl.h` and
  `amiga/gl2d_none.c` give the never-reached OpenGL calls something to
  link to; `amiga/stack.c` sets a 128 KB stack (512-byte strings several
  calls deep, and libnix's `fscanf`).

## Byte order

Every binary file read was checked:

- Replays (`.gjr`): the header's numbers are written and read byte by byte,
  big-end first, on every machine, so they were already right. The body is
  the game's memory buffer saved as it lies, so a PC's replays are
  little-endian; on the Amiga `repPush*` / `repGet*` now write and read it
  little-endian a byte at a time, so replays go between the two (and no
  word reads at odd addresses). Reads past the loaded body give 0; the
  hero count is kept to 4 and each hero's sprite and side to the tables'
  sizes, so a damaged replay cannot index past them; the file handle is
  closed when the version is wrong, and by `getReplayComment` (the replay
  menu opens every file in the drawer).
- PNGs and WAVs are read by acsdl12. Config, records and skin files are
  text.

State: builds and links with no warnings (`-Wno-unused-but-set-variable`
for upstream's leftovers, `-Wno-discarded-qualifiers` for `gettext.h`'s
no-NLS `_()`, which yields `const char *`). Not yet run in a lab or on a
real 68040.
