"""
Generates a C source file containing a bitmap font array for all printable ASCII characters.

This script uses a fixed-size monospace font to render each printable ASCII character (from
codepoint 32 to 126) into a 1-bit-per-pixel bitmap of configurable size (default: 10x10).
Each character's bitmap is flattened and written into a C array of the form:

    uint8_t charset[94][CHAR_WIDTH * CHAR_HEIGHT];

The output is saved as 'charset.c', and includes proper escaping for characters like '\\' and '\''.
This file is intended to be included by a kernel or low-level graphics system for text rendering.

Dependencies:
    - Pillow (PIL)
    - A monospace TrueType font (default: DejaVu Sans Mono)

Output:
    - A C source file at ../kernel/src/charset.c
"""

import os
from PIL import Image, ImageDraw, ImageFont

OUT_FILE: str = ""

FONT_SIZE: int = 10
CHAR_WIDTH: int = 10
CHAR_HEIGHT: int = 10
FONT_PATH: str = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"

chars: str = "".join([chr(x) for x in range(32, 127)])

def char_to_bitmap(c: str) -> list:
    """
    Converts a char to a bitmap showing it.

    args:
        c: str -> char to convert

    returns:
        list(img.getdata()) -> list with pixel values of the bitmap
    """
    img = Image.new(mode="1", size=(CHAR_WIDTH, CHAR_HEIGHT), color=0)

    font = ImageFont.truetype(FONT_PATH, FONT_SIZE)
    ImageDraw.Draw(img).text(xy=(0, -2), text=c, font=font, fill=1)

    return list(img.getdata())

def generate_file(fd) -> None:
    """
    Generates a file containing an array with the bitmaps of all printable ASCII-chars.

    args:
        fd -> file descriptor of the target file

    returns:
         /
    """

    fd.write("#include <stdint.h>\n\n")
    fd.write("#include <charset.h>\n\n")
    fd.write("uint8_t charset[95][CHAR_HEIGHT*CHAR_WIDTH] =\n{\n")

    for i, c in enumerate(chars):    
        # escapes special chars \ and ' -> otherwise the .c file would be broken
        if c == '\\' or c == '\'': fd.write("\t[CHARSET_INDEX('\\" + c + "')] = {\n")
        else: fd.write("\t[CHARSET_INDEX('" + c + "')] = {\n")

        # gets a bitmap of the char
        bitmap = char_to_bitmap(c)

        # writes the pixeldata of the bitmap in the file
        for y in range(0, CHAR_HEIGHT):
            fd.write("\t\t")
            for x in range(0, CHAR_WIDTH):
                fd.write(f"{bitmap[x+y*CHAR_WIDTH]}")
                # writes a ',' after every entry, except the last one
                if y < CHAR_HEIGHT-1 or x < CHAR_WIDTH-1: fd.write(",")
            fd.write("\n")
        
        if i != len(chars)-1: fd.write("\t},\n")
        else: fd.write("\t}\n")
    
    fd.write("};\n\n")

if __name__ == "__main__":
    SRC_DIR = os.path.abspath(os.path.dirname(__file__))
    OUT_FILE = f"{SRC_DIR}/../kernel/src/charset.c"

    with open(OUT_FILE, "w") as fd:
        generate_file(fd)