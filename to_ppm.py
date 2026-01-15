#!/usr/bin/env python3
import sys
from PIL import Image

if len(sys.argv) < 3:
    print(f"Usage: {sys.argv[0]} <input_image> <output.ppm>")
    sys.exit(1)

input_path = sys.argv[1]
output_path = sys.argv[2]

img = Image.open(input_path)
if img.mode != 'RGB':
    img = img.convert('RGB')

img.save(output_path, 'PPM')
print(f"Converted {input_path} -> {output_path}")
