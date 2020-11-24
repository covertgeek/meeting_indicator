#!/bin/bash

# <bitbar.title>Zoom Meeting Status</bitbar.title>
# <bitbar.version>v1.0</bitbar.version>
# <bitbar.author>covertgeek</bitbar.author>
# <bitbar.author.github>covertgeek</bitbar.author.github>
# <bitbar.desc>Detect if an active meeting is happening and display the meeting ID</bitbar.desc>

pythonenv="/Users/user/Documents/Bitbar/Plugins/ZoomMonitor"
script="main.py"

$pythonenv/venv/bin/python $pythonenv/$script
