#!/bin/sh
# Check for new mails
# Needs: mailcheck, awk
# No rights reserved
# Written by Jens Oliver John <jens.o.john@gmail.com>

while true; do
    mailcheck -c | awk '
        {
            mbox=$7
            gsub(/.+\//, "", mbox);
            entry=sprintf("twmnc -t \"%s\" -c \"%s %s message(s)\"", mbox, $3, $4);
            system(entry);
        }'
    sleep 300
done

