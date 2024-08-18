#!/bin/bash

./606-kick
./606-snare
./606-hats
./606-toms

flac -8 -e --no-padding -f 606-kick.wav
flac -8 -e --no-padding -f 606-snare.wav
flac -8 -e --no-padding -f 606-hat-closed.wav
flac -8 -e --no-padding -f 606-hat-open.wav
flac -8 -e --no-padding -f 606-tom-low.wav
flac -8 -e --no-padding -f 606-tom-high.wav
flac -8 -e --no-padding -f 606-cymbal.wav

cp -f ../resources/matsu-606.sfz matsu-606.sfz

zip -9 -D matsu.zip matsu-606.sfz \
606-kick.flac 606-snare.flac \
606-hat-closed.flac 606-hat-open.flac \
606-tom-low.flac 606-tom-high.flac \
606-cymbal.flac
