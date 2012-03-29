===========
WAVE Editor
===========

----------------------------------------------------
a command line program for editing WAVE audio files.
----------------------------------------------------

:Author: Meitian Huang <u4700480@gmail.com>
:Date:   2nd March, 2012
:Copyright: FreeBSD License
:Version: 0.3
:Manual Section: 1

SYNOPSIS
========
        audioedit -h
        audioedit -v
        audioedit OPTION -i FILE[...] -o FILE


DESCRIPTION
===========
``audioedit`` reads the input file(s) and generates desired output file to the
disk. Currently, ``audioedit`` supports two operations: trimming files and
joining files. In addition, it also supports WAVE playback.

OPTIONS
=======
    -h                  Display this help.
    -v                  Display the version number and built information (when
    possible).
    -tb n               Trim n samples from the beginning for the audio clip
    -te m               Trim m samples off the end of the audio clip
    -j                  Join the input files.
    -p                  Play the input files.
    -i file [...]       Specify the input file names.
    -o file             Specify the output file name(overwriting an existing
    file).

FILES
=====
The ``audioedit`` expects at least one input file and only one output file.
