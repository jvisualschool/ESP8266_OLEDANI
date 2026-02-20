import re

# Read base64 map
b64_map = {}
with open('thumbnails_base64.txt', 'r') as f:
    for line in f:
        line = line.strip()
        if '=' in line:
            var, b64 = line.split('=', 1)
            b64_map[var] = b64

cards = [
    ("idea", 1, "IDEA / BULB"),
    ("line_chart", 2, "LINE CHART"),
    ("meteor_rain", 3, "METEOR RAIN"),
    ("social_media", 4, "SOCIAL MEDIA"),
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

new_grid_html = '<div class="anim-grid">\n'
for var, num, title in cards:
    b64 = b64_map.get(var, "")
    active_class = ' active' if num == 1 else ''
    new_grid_html += f'                <div class="anim-card{active_class}" data-anim="{num}" onclick="selectAnim(this)">\n'
    new_grid_html += f'                    <img class="anim-img" src="data:image/gif;base64,{b64}">\n'
    new_grid_html += f'                    <span>{title}</span>\n'
    new_grid_html += '                </div>\n'
new_grid_html += '            </div>\n            <input'

with open('src/main.cpp', 'r') as f:
    content = f.read()

# Using regex to find the section to replace: from <label>ANIMATION STYLE</label>... missing grid to <input type="hidden" id="anim" value="1">
# It currently matches:
pattern = re.compile(r'<label>ANIMATION STYLE</label>.*?(<input type="hidden" id="anim")', re.DOTALL)
replacement = '<label>ANIMATION STYLE</label>\n            ' + new_grid_html

new_content = pattern.sub(replacement, content)

with open('src/main.cpp', 'w') as f:
    f.write(new_content)

