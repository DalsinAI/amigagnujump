#!/usr/bin/env python3
"""skin8: GNUjump's skins in 256 colours for acsdl12.

acsdl12 draws on an 8-bit screen. Its PNG loader keeps a picture's own
palette when it has 256 colours or fewer and cuts anything richer down to
3-3-2, and a blit maps each picture's palette onto the screen's. GNUjump's
skins are RGBA PNGs, the menu and game backgrounds with tens of thousands of
colours, so on the Amiga they would come out in 3-3-2 and every blit would
go through a colour map.

This gives each skin one palette of its own, made from all of its pictures
together, and writes every picture as a paletted PNG in that palette:

  index 0      black, and the colour key (tRNS 0): pixels under half alpha
  index 1      magenta (255, 0, 255), SFont's character markers, kept exact
  index 2-255  the skin's colours (median cut, libimagequant when Pillow has
               it), backgrounds dithered, sprites and fonts not

and palette.png, 16x16 pixels of indices 0-255, which the game loads to set
the screen's palette when it loads the skin (amiga/amiga.c). With the
screen in the skin's palette, blits copy without a map.

  python3 amiga/tools/skin8.py skins amiga/skins

Needs Pillow and numpy. The output is committed (amiga/skins), so the
Amiga build itself needs neither.

The AmiCradle Team. GPL 3, like GNUjump.
"""

import os
import shutil
import sys

import numpy as np
from PIL import Image, features

KEY = 0
MAGENTA = 1
FIRST = 2
NCOLOURS = 256 - FIRST
BIG = 200 * 200          # pictures this size or larger are dithered
SKIP = {"Makefile.am", "Makefile.in"}


def load(path):
    im = Image.open(path)
    im.load()
    return np.asarray(im.convert("RGBA"), dtype=np.uint8)


def skin_palette(pictures):
    """The skin's colours from all its opaque pixels, small pictures weighted
    up so a sprite's few colours are not lost to the backgrounds'."""
    rows = []
    for rgba in pictures.values():
        px = rgba.reshape(-1, 4)
        px = px[px[:, 3] >= 128][:, :3]
        px = px[~((px[:, 0] == 255) & (px[:, 1] == 0) & (px[:, 2] == 255))]
        if not len(px):
            continue
        repeat = max(1, min(32, 16384 // len(px)))
        rows.append(np.repeat(px, repeat, axis=0))
    sample = np.concatenate(rows)
    width = 1024
    pad = (-len(sample)) % width
    sample = np.concatenate([sample, np.repeat(sample[:1], pad, axis=0)])
    im = Image.fromarray(sample.reshape(-1, width, 3), "RGB")
    method = (Image.Quantize.LIBIMAGEQUANT if features.check("libimagequant")
              else Image.Quantize.MEDIANCUT)
    q = im.quantize(colors=NCOLOURS, method=method, dither=Image.Dither.NONE)
    pal = np.array(q.getpalette()[:3 * NCOLOURS], dtype=np.uint8).reshape(-1, 3)
    used = sorted(set(np.unique(np.asarray(q)).tolist()))
    colours = [tuple(int(v) for v in pal[i]) for i in used]
    colours = sorted(set(colours))
    while len(colours) < NCOLOURS:
        colours.append(colours[-1])
    return colours


def full_palette(colours):
    return [(0, 0, 0), (255, 0, 255)] + list(colours)


def to_indices(rgba, colours):
    h, w = rgba.shape[:2]
    rgb = Image.fromarray(np.ascontiguousarray(rgba[:, :, :3]), "RGB")
    palimg = Image.new("P", (1, 1))
    flat = []
    for c in colours:
        flat.extend(c)
    palimg.putpalette(flat)
    dither = Image.Dither.FLOYDSTEINBERG if h * w >= BIG else Image.Dither.NONE
    q = np.asarray(rgb.quantize(palette=palimg, dither=dither), dtype=np.int32)
    # Pillow's index into `colours` -> ours; any index past the list (Pillow
    # pads palettes) goes to the nearest real entry, which is the first.
    q = np.where(q < len(colours), q, 0) + FIRST
    out = q.astype(np.uint8)
    magenta = (rgba[:, :, 0] == 255) & (rgba[:, :, 1] == 0) & (rgba[:, :, 2] == 255)
    out[magenta & (rgba[:, :, 3] >= 128)] = MAGENTA
    out[rgba[:, :, 3] < 128] = KEY
    return out


def write_png(path, indices, palette, keyed):
    im = Image.fromarray(indices, "P")
    flat = []
    for c in palette:
        flat.extend(c)
    im.putpalette(flat)
    kw = {"optimize": False}
    if keyed:
        kw["transparency"] = bytes([0] + [255] * 255)
    im.save(path, **kw)


def convert_skin(src, dst):
    pictures = {}
    for root, _dirs, files in os.walk(src):
        for name in sorted(files):
            if name.lower().endswith(".png"):
                pictures[os.path.join(root, name)] = load(os.path.join(root, name))
    colours = skin_palette(pictures)
    palette = full_palette(colours)
    for root, _dirs, files in os.walk(src):
        rel = os.path.relpath(root, src)
        outdir = os.path.normpath(os.path.join(dst, rel))
        os.makedirs(outdir, exist_ok=True)
        for name in sorted(files):
            if name in SKIP:
                continue
            path = os.path.join(root, name)
            out = os.path.join(outdir, name)
            if path in pictures:
                rgba = pictures[path]
                write_png(out, to_indices(rgba, colours), palette,
                          bool((rgba[:, :, 3] < 128).any()))
            else:
                shutil.copyfile(path, out)
    write_png(os.path.join(dst, "palette.png"),
              np.arange(256, dtype=np.uint8).reshape(16, 16), palette, False)
    print(f"{src}: {len(pictures)} pictures, {len(set(colours))} colours")


def main():
    if len(sys.argv) != 3:
        sys.exit("usage: skin8.py SKINS_DIR OUT_DIR")
    src, dst = sys.argv[1], sys.argv[2]
    for skin in sorted(os.listdir(src)):
        if os.path.isdir(os.path.join(src, skin)):
            convert_skin(os.path.join(src, skin), os.path.join(dst, skin))


if __name__ == "__main__":
    main()
