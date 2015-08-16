#!/bin/sh
# Checks if disk usage goes over a certain %
# Needs: df, awk
# No rights reserved
# Written by Charles-William Crete <me@charles-william.me>


# If you want to check usage for only certain disk (in this case /dev/sda and /dev/sdb,
#  use this command. You can change the "[ab]" with your own disk
#  ([cd] for /dev/sdc and /dev/sdd by example)
#  df -hl | awk '/^\/dev\/sd[ab]/ { sum+=$5 } END { print sum }'
# Default will check all drives

# Max % that the disk can do until notifying you.
MAX_USAGE=5

# Time in seconds between checks/messages.
#  Common values:
#  300  = 5  mins
#  600  = 10 mins
#  1800 = 30 mins
#  3600 = 1  hour

TIME_IN_BETWEEN=300

while true; do

    # Get % of disk usage
    USAGE=`df -hl | awk '/^\/dev\/sd/ { sum+=$5 } END { print sum }'`

    # If current usage is higher than max usage
    if [[ $USAGE -gt $MAX_USAGE ]] ; then
        twmnc --title "Disk Usage High" --content "Reached ${USAGE}%"
    fi

    # Wait until next check/message
    sleep $TIME_IN_BETWEEN
done

