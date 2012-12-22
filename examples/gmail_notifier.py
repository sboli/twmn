#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright 2011 Orphée Lafond-Lummis <orftz.com>. No rights reserved.

"""Gmail notifier for twmn.

1. Fill in your email and password.
2. Enable IMAP in Gmail.
3. Enjoy.
"""

#TODO: handle disconnection
#TODO: store password in a less sucky way (getpass maybe?)
#TODO: add gmail icon to notification request

import imaplib
import re
import socket
import time
import os

def cred_from_muttrc():
    with open(os.path.expanduser('~/.muttrc')) as fin:
        data = fin.read()
    re_user = re.search(r'set\s+imap_user\s*=\s*"(.*)"', data, re.MULTILINE)
    re_pass = re.search(r'set\s+imap_pass\s*=\s*"(.*)"', data, re.MULTILINE)
    if re_user:
        user = re_user.group(1)
    if re_pass:
        password = re_pass.group(1)
    return user, password


# Options
# EMAIL = "USERNAME@gmail.com"
# PASSWORD = "SECRET"
# ... or if ~/.muttrc exists, read it.
EMAIL, PASSWORD = cred_from_muttrc()
FREQUENCY = 30 * 60  # Check emails every 30 minutes
TWMN_PORT = 9797
TWNM_ADDR = "127.0.0.1"


def notification(sock, content):
    TITLE = "Gmail"
    message = "<root>" + \
              "<title>{}</title>".format(TITLE) + \
              "<content>{}</content>".format(content) + \
              "<icon>{}</icon>".format("email_icon") + \
              "</root>"
    sock.send(bytes(message.encode("utf-8")))


if __name__ == "__main__":
    # Connect to twmn
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect((TWNM_ADDR, TWMN_PORT))

    # Login to Gmail and select inbox
    try:
        gmail = imaplib.IMAP4_SSL('imap.gmail.com', 993)
        gmail.login(EMAIL, PASSWORD)
        gmail.select(readonly=1)  # selects inbox by default
    except imaplib.IMAP4.error as err:
        notification(s, "Couldn't connect properly.")
        raise SystemExit(-1)

    # Check emails at FREQUENCY interval
    while True:
        unreadCount = re.search(b"UNSEEN (\d+)", \
                            gmail.status("INBOX", "(UNSEEN)")[1][0]).group(1)
        unreadCount = int(unreadCount)
        if unreadCount > 0:
            content = "{} unread".format(unreadCount) + \
                      " email{}.".format(("s" if unreadCount > 1 else ""))
            notification(s, content)
        time.sleep(FREQUENCY)
