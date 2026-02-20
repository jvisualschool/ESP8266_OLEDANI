import serial, time
try:
    s = serial.Serial('/dev/cu.usbserial-10', 74880, timeout=1)
    for _ in range(10):
        print(s.readline().decode('utf-8', errors='replace').strip())
except Exception as e:
    print("Error:", e)
