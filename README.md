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

# DESIGN
1. Functions should be atomic. Functions must have a minimal amount of side
   effects.

2. The returned value of functions MUST be checked. Programmers who do not 
   check the return value of fread() are ugly and stupid.

3. Compatibility with Microsoft's OS is the last TODO.

4. Using bloated libraries is stupid.

5. The program should be defensive.

6. Your code is shit if there is ONE function that has an argument such that:
   a) it is not a pointer, b) it is not declared as `const`.

7. If you write a program of this size and there is no single function that has
   at least one `const` argument, GO TO HELL.
