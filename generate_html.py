import sys

# read base64 lines
b64_map = {}
with open('thumbnails_base64.txt', 'r') as f:
    for line in f:
        line = line.strip()
        if '=' in line:
            var, b64 = line.split('=', 1)
            b64_map[var] = b64

cards = [
    ("bar_chart", 5, "BAR CHART"),
    ("book", 6, "BOOK"),
    ("calendar", 7, "CALENDAR"),
    ("cloud_network", 8, "CLOUD NET"),
    ("fingerprint_scan", 9, "FINGER PRINT"),
    ("home", 10, "HOME"),
    ("hot", 11, "HOT"),
    ("in_love", 12, "IN LOVE"),
    ("laptop", 13, "LAPTOP"),
    ("location", 14, "LOCATION"),
    ("map1", 15, "MAP 1"),
    ("map2", 16, "MAP 2"),
    ("monitor", 17, "MONITOR"),
    ("photo_camera", 18, "CAMERA"),
    ("suitcase", 19, "SUITCASE"),
    ("worldwide", 20, "WORLD")
]

for var, num, title in cards:
    b64 = b64_map.get(var, "")
    print(f'                <div class="anim-card" data-anim="{num}" onclick="selectAnim(this)">')
    print(f'                    <img class="anim-img" src="data:image/gif;base64,{b64}">')
    print(f'                    <span>{title}</span>')
    print('                </div>')

