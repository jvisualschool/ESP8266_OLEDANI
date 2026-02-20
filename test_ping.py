import urllib.request, time

def try_req():
    try:
        urllib.request.urlopen("http://billboard.local", timeout=2)
        print("Success root")
    except Exception as e:
        print("Error root:", e)

    try:
        urllib.request.urlopen("http://billboard.local/update?msg=TEST&anim=1", timeout=2)
        print("Success update")
    except Exception as e:
        print("Error update:", e)

try_req()
