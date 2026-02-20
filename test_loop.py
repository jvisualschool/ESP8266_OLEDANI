import urllib.request
import time
while True:
    try:
        urllib.request.urlopen("http://192.168.0.111/update?msg=TEST&anim=1", timeout=1)
        print("Success")
    except Exception as e:
        print("Error:", e)
    time.sleep(2)
