#!/usr/bin/env python3
"""
Convert 16 new GIFs to C header files for ESP8266 OLED display.
Format: 64x64 pixels, 512 bytes/frame, MSB-first, dark pixel = bit 1.
"""

from PIL import Image
import os
import sys

BASE_DIR = "/Users/jinhojung/Desktop/ESP8266_OLED_ANI"
INCLUDE_DIR = os.path.join(BASE_DIR, "include")

GIF_NAMES = [
    "bar-chart",
    "book",
    "calendar",
    "cloud-network",
    "fingerprint-scan",
    "home",
    "hot",
    "in-love",
    "laptop",
    "location",
    "map1",
    "map2",
    "monitor",
    "photo-camera",
    "suitcase",
    "worldwide",
]

def gif_name_to_var(gif_name):
    """Convert gif filename (without .gif) to C variable name."""
    return gif_name.replace("-", "_")

def gif_to_header(gif_path, output_path, var_name):
    """Convert a GIF file to a C header file."""
    upper_name = var_name.upper()

    img = Image.open(gif_path)
    n_frames = img.n_frames
    print(f"  Processing {os.path.basename(gif_path)}: {n_frames} frames, size {img.size}")

    lines = []
    guard = f"ANIMATION_{upper_name}_H"
    lines.append(f"#ifndef {guard}")
    lines.append(f"#define {guard}")
    lines.append("")
    lines.append("#include <pgmspace.h>")
    lines.append("")
    lines.append(f"#define {upper_name}_FRAMES {n_frames}")
    lines.append(f"#define {upper_name}_WIDTH  64")
    lines.append(f"#define {upper_name}_HEIGHT 64")
    lines.append("")
    lines.append(f"const uint8_t {var_name}_frames[][512] PROGMEM = {{")

    for frame_idx in range(n_frames):
        img.seek(frame_idx)
        
        # Convert frame to RGBA to handle transparency
        frame = img.convert("RGBA")
        
        # Ensure all colors map to black and all transparency/white map to white
        # BEFORE resizing. This prevents dirty/noisy anti-aliasing dots.
        raw_data = frame.getdata()
        new_data = []
        for r, g, b, a in raw_data:
            if a < 128:
                new_data.append((255, 255, 255, 255)) # Transparent -> White
            elif r > 240 and g > 240 and b > 240:
                new_data.append((255, 255, 255, 255)) # White -> White
            else:
                new_data.append((0, 0, 0, 255))       # Any other color -> Black
                
        clean_frame = Image.new("RGBA", frame.size)
        clean_frame.putdata(new_data)
        
        frame_rgb = clean_frame.convert("RGB")

        # Resize to 64x64 using LANCZOS for smooth downscaling
        frame_64 = frame_rgb.resize((64, 64), Image.LANCZOS)

        # Convert to grayscale
        frame_gray = frame_64.convert("L")
        raw_pixels = list(frame_gray.getdata())

        # Binarize: 1 = dark (OLED white), 0 = light (OLED black)
        bin_pixels = [1 if px < 160 else 0 for px in raw_pixels]

        # Remove single isolated dots (noise filtering)
        clean_pixels = list(bin_pixels)
        for y in range(64):
            for x in range(64):
                if bin_pixels[y * 64 + x] == 1:
                    neighbors = 0
                    for dy in [-1, 0, 1]:
                        for dx in [-1, 0, 1]:
                            if dx == 0 and dy == 0: continue
                            nx, ny = x + dx, y + dy
                            if 0 <= nx < 64 and 0 <= ny < 64:
                                neighbors += bin_pixels[ny * 64 + nx]
                    if neighbors == 0:
                        clean_pixels[y * 64 + x] = 0  # Erase dot

        # Pack bits: MSB first
        byte_data = []
        for byte_idx in range(512):
            byte_val = 0
            for bit in range(8):
                pixel_idx = byte_idx * 8 + bit
                if clean_pixels[pixel_idx] == 1:
                    byte_val |= (1 << (7 - bit))
            byte_data.append(byte_val)

        # Format as hex bytes, 16 per line
        lines.append("  {")
        row_parts = []
        for i in range(0, 512, 16):
            chunk = byte_data[i:i+16]
            hex_str = ", ".join(f"0x{b:02X}" for b in chunk)
            row_parts.append(f"    {hex_str}, ")
        lines.extend(row_parts)
        lines.append("      },")

    lines.append("};")
    lines.append("")
    lines.append("#endif")

    with open(output_path, "w") as f:
        f.write("\n".join(lines) + "\n")

    print(f"  -> Wrote {output_path}")

def main():
    os.makedirs(INCLUDE_DIR, exist_ok=True)

    for gif_name in GIF_NAMES:
        gif_path = os.path.join(BASE_DIR, f"{gif_name}.gif")
        if not os.path.exists(gif_path):
            print(f"WARNING: {gif_path} not found, skipping.")
            continue
        var_name = gif_name_to_var(gif_name)
        output_path = os.path.join(INCLUDE_DIR, f"animation_{var_name}.h")
        print(f"\n[{gif_name}]")
        gif_to_header(gif_path, output_path, var_name)

    print("\nDone! All header files generated.")

if __name__ == "__main__":
    main()
