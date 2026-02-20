#!/usr/bin/env python3
"""
Generate 48x48 thumbnail GIFs and base64 encode them.
- Single frame from the middle of the animation
- 16 colors, white background
- All 20 animations (16 new + 4 existing)
"""

from PIL import Image
import os
import io
import base64

BASE_DIR = "/Users/jinhojung/Desktop/ESP8266_OLED_ANI"

# All 20 animations: (gif_filename_base, var_name)
ALL_ANIMATIONS = [
    # 16 new
    ("bar-chart",       "bar_chart"),
    ("book",            "book"),
    ("calendar",        "calendar"),
    ("cloud-network",   "cloud_network"),
    ("fingerprint-scan","fingerprint_scan"),
    ("home",            "home"),
    ("hot",             "hot"),
    ("in-love",         "in_love"),
    ("laptop",          "laptop"),
    ("location",        "location"),
    ("map1",            "map1"),
    ("map2",            "map2"),
    ("monitor",         "monitor"),
    ("photo-camera",    "photo_camera"),
    ("suitcase",        "suitcase"),
    ("worldwide",       "worldwide"),
    # 4 existing
    ("idea",            "idea"),
    ("line-chart",      "line_chart"),
    ("meteor-rain",     "meteor_rain"),
    ("social-media",    "social_media"),
]

def make_thumbnail_b64(gif_path, var_name):
    """Create a 48x48 thumbnail GIF (middle frame, 16 colors, white bg) and return base64."""
    img = Image.open(gif_path)
    n_frames = img.n_frames
    mid_frame = n_frames // 2

    img.seek(mid_frame)

    # Composite onto white background
    frame = img.convert("RGBA")
    bg = Image.new("RGBA", frame.size, (255, 255, 255, 255))
    bg.paste(frame, mask=frame.split()[3])
    frame_rgb = bg.convert("RGB")

    # Resize to 48x48 using LANCZOS
    thumb = frame_rgb.resize((48, 48), Image.LANCZOS)

    # Quantize to 16 colors with white background
    thumb_p = thumb.quantize(colors=16, method=Image.Quantize.MEDIANCUT)

    # Save as GIF to buffer
    buf = io.BytesIO()
    thumb_p.save(buf, format="GIF", optimize=True)
    buf.seek(0)

    b64 = base64.b64encode(buf.read()).decode("ascii")
    return b64

results = []
for gif_base, var_name in ALL_ANIMATIONS:
    gif_path = os.path.join(BASE_DIR, f"{gif_base}.gif")
    if not os.path.exists(gif_path):
        print(f"WARNING: {gif_path} not found, skipping.")
        continue
    b64 = make_thumbnail_b64(gif_path, var_name)
    results.append(f"{var_name}={b64}")
    print(f"Processed: {var_name}")

print("\n--- BASE64 THUMBNAILS ---")
for line in results:
    print(line)
