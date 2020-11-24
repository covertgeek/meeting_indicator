# Script written by Derek Rushing
# Last Update: November 23, 2020
# License: MIT

from pathlib import Path

import logging
import os
import subprocess

from requests.adapters import HTTPAdapter
from requests.packages.urllib3.util.retry import Retry
import requests

log = logging.getLogger()

esp32 = "http://board_ip"


# Original Source: https://www.peterbe.com/plog/best-practice-with-retries-with-requests
# Pulled function to handle connection errors/timeouts
def requests_retry_session(
    retries=3,
    backoff_factor=0.3,
    status_forcelist=(500, 502, 504),
    session=None,
):
    session = session or requests.Session()
    retry = Retry(
        total=retries,
        read=retries,
        connect=retries,
        backoff_factor=backoff_factor,
        status_forcelist=status_forcelist,
    )
    adapter = HTTPAdapter(max_retries=retry)
    session.mount('http://', adapter)
    session.mount('https://', adapter)
    return session


# Get the matching process
p1 = subprocess.Popen(['ps', 'x'], stdout=subprocess.PIPE)
p2 = subprocess.Popen(["grep", "-i", "zoom.us"], stdin=p1.stdout, stdout=subprocess.PIPE)
p3 = subprocess.Popen(
    ["grep", "-E", "\-key [0-9]{10,10}"], stdin=p2.stdout, stdout=subprocess.PIPE)
p1.stdout.close()
p2.stdout.close()

output = p3.communicate()[0]

if output:
    args = output.decode().split()
    for item in args:
        if item == "-pid":
            pid = args[args.index(item)-len(args) + 1]

    # Attempt to open the crashlog corresponding to the PID of the process
    logfile = Path.joinpath(Path.home(), "Library",
                            "Logs", "zoom.us", "crashlog", f"{pid}.log")

    if os.path.exists(logfile):
        meeting_id = "unknown"
        logdata = open(logfile, 'r').readlines()
        logdata.reverse()

        # Parse through the log and find the most recent meeting-id
        for line in logdata:
            try:
                key, value = line.split(":", 1)
                if key == "meeting-id":
                    meeting_id = value
                    break
            except ValueError:
                pass
        
        print(f"Zoom Meeting # {value}") 
    else:
        # If the log doesn't exist, just use the key
        code = output.split()[-1].decode()
        print("Zoom Meeting # ", str(code))
    try:
        requests_retry_session().get(f"{esp32}/meeting/on")
    except Exception as e:
        log.error(f"Unable to set meeting status as on.  Reason was {e}")
else:
    try:
        requests_retry_session().get(f"{esp32}/meeting/off")
    except Exception as e:
        log.error(f"Unable to set meeting status as off.  Reason was {e}")
    print("Avail.")
