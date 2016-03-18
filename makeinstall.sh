#!/bin/bash
#

rm -r /Applications/Sowicm\'s\ Hourglass.app
cp -r ~/tmp/build-Hourglass-Desktop_Qt_5_6_0_clang_64bit-Release/Hourglass.app /Applications/Sowicm\'s\ Hourglass.app
defaults write /Applications/Sowicm\'s\ Hourglass.app/Contents/Info LSUIElement -string 1
