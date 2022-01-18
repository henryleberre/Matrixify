#!/usr/bin/python
# -*- coding: utf-8 -*-
from PIL import Image, ImageDraw, ImageFont, ImageFilter

import random

#configuration
IMG_SIZE=500

idx = 0

FONT_NAMES  = ["DejaVuSansMono-Bold.ttf", "misc/katakana.ttf", "misc/katakana_pipe.ttf"]
FONT_RANGES = [(0x21, 0x7E),              (32, 126),           (32, 126)]

FONTS = []
for font_name in FONT_NAMES:
    FONTS.append(ImageFont.truetype(font_name, IMG_SIZE))

for i in range(len(FONTS)):
    FONT = FONTS[i]

    for j in range(FONT_RANGES[i][0], FONT_RANGES[i][1]):
        char = chr(j)

        tmp = Image.new("RGBA", (IMG_SIZE,IMG_SIZE), (0,0,0,0))
        ImageDraw.Draw(tmp).text((0, 0), char, font=FONT, fill=(0,0,0, 255))
        tmp_pix = tmp.load()

        char_l = 0
        for i in range(IMG_SIZE):
            bFoundPixel = False
            for j in range(IMG_SIZE):
                if tmp_pix[i, j][3]>0:
                    bFoundPixel = True
                    break
            if bFoundPixel:
                break
            char_l = char_l + 1
        
        char_u = 0
        for i in range(IMG_SIZE):
            bFoundPixel = False
            for j in range(IMG_SIZE):
                if tmp_pix[j, i][3]>0:
                    bFoundPixel = True
                    break
            if bFoundPixel:
                break
            char_u = char_u + 1
        
        tmp = Image.new("RGBA", (IMG_SIZE,IMG_SIZE), (0,0,0,0))
        ImageDraw.Draw(tmp).text((-char_l, -char_u), char, font=FONT, fill=(0,0,0, 255))
        tmp_pix = tmp.load()

        char_w = 0
        for i in range(IMG_SIZE):
            bFoundPixel = False
            for j in range(IMG_SIZE):
                if tmp_pix[i, j][3]>0:
                    bFoundPixel = True
                    break
            if not bFoundPixel:
                break
            char_w = char_w + 1
        
        char_h = 0
        for i in range(IMG_SIZE):
            bFoundPixel = False
            for j in range(IMG_SIZE):
                if tmp_pix[j, i][3]>0:
                    bFoundPixel = True
                    break
            if not bFoundPixel:
                break
            char_h = char_h + 1

        print(idx, char_w, char_h)

        im = Image.new("L", (IMG_SIZE,IMG_SIZE), (0))
        
        xy = (
            int(-char_l + IMG_SIZE/2 - char_w/2),
            int(-char_u + IMG_SIZE/2 - char_h/2)
        )

        ImageDraw.Draw(im).text(xy, char, font=FONT, fill=(255))

        im.save("temp/input_characters/char_"+str(idx)+".png")

        idx = idx + 1