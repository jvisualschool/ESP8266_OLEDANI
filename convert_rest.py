import os
import glob
import re

anim_names = ['line_chart', 'meteor-rain', 'social-media']

for name in anim_names:
    files = glob.glob(f'frames/f_{name}_*.pbm')
    # sorting by the sequence number e.g. f_line-chart_000.pbm -> 0, f_line-chart_000-1.pbm -> ...
    # wait, due to coalescing gif, they will just be f_xxxx_000.pbm or f_xxxx_000-0.pbm
    
    # We can just extract all digits from filename
    def sort_key(x):
        nums = re.findall(r'\d+', x)
        return [int(n) for n in nums]
    
    files.sort(key=sort_key)
    
    var_name = name.replace('-', '_')
    upper_name = var_name.upper()

    out_h = open(f'include/animation_{var_name}.h', 'w')
    out_h.write(f'#ifndef ANIMATION_{upper_name}_H\n')
    out_h.write(f'#define ANIMATION_{upper_name}_H\n\n')
    out_h.write('#include <pgmspace.h>\n\n')

    out_h.write(f'#define {upper_name}_FRAMES {len(files)}\n')
    out_h.write(f'#define {upper_name}_WIDTH  64\n')
    out_h.write(f'#define {upper_name}_HEIGHT 64\n\n')

    out_h.write(f'const uint8_t {var_name}_frames[][512] PROGMEM = {{\n')

    for file_idx, fpath in enumerate(files):
        with open(fpath, 'r') as f:
            content = f.read().split()
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
            pixel_str = "".join(pixels)
            
            byte_array = []
            for y in range(height):
                for x_byte_idx in range(width // 8):
                    byte_val = 0
                    for bit in range(8):
                        x = x_byte_idx * 8 + bit
                        pixel_idx = y * width + x
                        p = int(pixel_str[pixel_idx]) if pixel_idx < len(pixel_str) else 0
                        if p:
                            byte_val |= (1 << (7 - bit))
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
    print(f"Header file generated successfully for {name}. Total frames: {len(files)}")
