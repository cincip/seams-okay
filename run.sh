#!/bin/bash

if [ $# -lt 2 ]; then
    echo "Usage: $0 <input_image> <num_seams>"
    echo "Example: $0 image.jpg 50"
    exit 1
fi

input_image=$1
num_seams=$2

base=$(basename "$input_image" | sed 's/\.[^.]*$//')

echo "[1/4] Compiling seam carving..."
gcc -O2 -lm seam.c -o seam || exit 1

echo "[2/4] Converting to PPM..."
python3 to_ppm.py "$input_image" "$base.ppm" || exit 1

echo "[3/4] Running seam carving ($num_seams seams)..."
./seam "$base.ppm" "${base}_carved.ppm" "$num_seams" || exit 1

echo "[4/4] Converting output to PNG..."
python3 << EOF
from PIL import Image
img = Image.open('${base}_carved.ppm')
img.save('${base}_carved.png')
print(f"Done! Output saved to ${base}_carved.png")
EOF
