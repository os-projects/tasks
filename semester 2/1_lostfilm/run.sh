#!/bin/bash

width=1366
height=768
depth=16

DISPLAY=:1 firefox -width $width -height $height http://lostfilm.tv &
Xvfb :1 -screen 0 ${width}x${height}x${depth} &

sleep 10

DISPLAY=:1 import -window root ./before.jpg  # make screenshot

DISPLAY=:1 xdotool mousemove $(python main.py before.jpg)
DISPLAY=:1 xdotool click 1

sleep 6

DISPLAY=:1 import -window root ./after.jpg  # screenshot again

for process in firefox Xvfb
do
	for pid in `ps -A | grep $process`
	do
		kill $pid; break; 
	done
done
