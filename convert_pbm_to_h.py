import os
import glob
import re

# Get all pbm files
files = glob.glob('frames/f_%03d-*.pbm')
files.sort(key=lambda x: int(re.search(r'-(\d+)\.pbm', x).group(1)))

out_h = open('include/animation_idea.h', 'w')
out_h.write('#ifndef ANIMATION_IDEA_H\n')
out_h.write('#define ANIMATION_IDEA_H\n\n')
out_h.write('#include <pgmspace.h>\n\n')

out_h.write(f'#define IDEA_FRAMES {len(files)}\n')
out_h.write('#define IDEA_WIDTH  64\n')
out_h.write('#define IDEA_HEIGHT 64\n\n')

# Store pointer to each frame
out_h.write('const uint8_t idea_frames[][512] PROGMEM = {\n')

for file_idx, fpath in enumerate(files):
    with open(fpath, 'r') as f:
        # read lines, skip comments and read header
        content = f.read().split()
        # content[0] should be P1, followed by width and height, then pixels
        idx = 0
        if content[idx] == 'P1':
            idx += 1
        else:
            print(f"Error: Not P1 format in {fpath}")
            continue
            
        width = int(content[idx])
        height = int(content[idx+1])
        idx += 2
        
        pixels = content[idx:]
        
        # PBM: 1 is black, 0 is white. For OLED, white is "on", so 0 should be lit?
        # Typically white background in gif means 0 in pbm? Let's assume 0 means pixel off, 1 means pixel on. 
        # Actually in PBM: 1 means black ink, 0 means white background.
        # So we might want 1 to be OLED on (white) if the image lines are black in gif.
        # Let's see later.
        
        pixel_str = "".join(pixels)
        if len(pixel_str) < width*height:
            print(f"Warning: Not enough pixels in {fpath}")

        byte_array = []
        for y in range(height):
            for x_byte_idx in range(width // 8):
                byte_val = 0
                for bit in range(8):
                    x = x_byte_idx * 8 + bit
                    pixel_idx = y * width + x
                    # Get pixel, default 0 if out of bounds
                    p = int(pixel_str[pixel_idx]) if pixel_idx < len(pixel_str) else 0
                    
                    # For OLED, normally we want the original non-background part to be lit.
                    # PBM gives 1 for black, 0 for white. Let's invert if background is mostly white. 
                    # If p == 1, we set bit to 1. 
                    if p:
                        byte_val |= (1 << (7 - bit)) # Adafruit drawBitmap uses MSB first
                byte_array.append(byte_val)
        
        out_h.write('  {\n    ')
        for i, b in enumerate(byte_array):
            out_h.write(f'0x{b:02X}, ')
            if (i+1) % 16 == 0:
                out_h.write('\n    ')
        out_h.write('  },\n')

out_h.write('};\n\n')
out_h.write('#endif\n')
out_h.close()
print("Header file generated successfully.")
