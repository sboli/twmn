#!/usr/bin/env python2

import socket, mpd, time

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.connect(("127.0.0.1", 9797))
client = mpd.MPDClient()
client.connect("127.0.0.1", "6600")
prev = ""
while True:
    current = client.currentsong()
    if prev != current:
        s.send("<root><title>" + current["title"] + "</title>"
	       "<content> from " + current["artist"] + "</content></root>")
        prev = current
    time.sleep(1)
