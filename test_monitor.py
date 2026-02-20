import serial, time
try:
    s = serial.Serial('/dev/cu.usbserial-10', 74880, timeout=1)
except Exception as e:
    print(f"Failed to open port: {e}")
    exit(1)

import urllib.request
try:
    urllib.request.urlopen('http://192.168.0.111/update?msg=TEST&anim=1', timeout=2)
except Exception as e:
    print(f"Failed to fetch: {e}")

end_time = time.time() + 5
while time.time() < end_time:
    line = s.readline()
    if line:
        print(line.decode(errors='replace').strip())
