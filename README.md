% AUDIOEDIT(1) Linux User Mannuals
% Meitian Huang
% 28 March, 2012

# NAME
audioeditor - command line program for editing WAVE audio files.

#SYNOPSIS
``audioedit -h``

``audioedit -v``

``audioedit *OPTION* -i FILE [...] -o FILE``


# DESCRIPTION
``audioedit`` reads the input file(s) and generates desired output file to the
disk. Currently, ``audioedit`` supports two operations: trimming files and
joining files. In addition, it also supports WAVE playback.

# OPTIONS
-h
:    Display this help.

-v
:    Display the version number and built information (when
    possible).

-tb *n*
:    Trim *n* samples from the beginning for the audio clip

-te *m*
:    Trim *m* samples off the end of the audio clip

-j
:    Join the input files.

-p
:    Play the input files.

-i *FILE* [...]
:    Specify the input file names.

-o *FILE*
:    Specify the output file name(overwriting an existing file).

# FILE
``audioedit`` expects at least one input file and only one output file.
