NAME
====
audioeditor - command line program for editing WAVE audio files.

SYNOPSIS
--------

        audioedit -h

        audioedit -v

        audioedit -p FILE

        audioedit *OPTION* -i FILE [FILE] -o FILE


DESCRIPTION
-----------
``audioedit`` reads the input file(s) and generates desired output file to the
disk. Currently, ``audioedit`` supports three operations: trimming, joining, and
merging. In addition, it also supports WAVE playback.

OPTIONS
-------

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

-m
:    Merge the input files.

-p
:    Play the input files.

-i *FILE* [...]
:    Specify the input file names.

-o *FILE*
:    Specify the output file name(overwriting an existing file).

FILE
----
``audioedit`` expects different numbers of FILEs upon different operations. To
be specific, it expects:

- Exactly 1 input file for trimming.

- Exactly 1 input file for playing.

- Exactly 2 input files for merging and joining.

EXAMPLES
--------

- Trim the first 100000 samples and the last 100000 samples of the file
  ``foo.wav`` and save the resultant file to ``bar.wav``.

        audioedit -tb 100000 -te 100000 -i foo.wav -o bar.wav

- Join foo.wav with bar.wav and save to foobar.wav.

        audioedit -j -i foo.wav bar.wav -o foobar.wav

- Merge foo.wav with bar.wav and save to foobar.wav.

        audioedit -m -i foo.wav bar.wav -o foobar.wav

- Play foo.wav.

        audioedit -p foo.wav

BUGS
----
- This program cannot directly play audio on the latest version of Ubuntu
  because the playback depends on OSS while Ubuntu has switched to
  Pulse(alsa-based). Instead, use:
        padsp audioedit -p foo.wav
  if you want to play the audio.

- This program cannot play audio on Mac OS X because Mac OS X has its own set
  of APIs(for example, QuickTimes).

- Merging is only possible for files that have bit-per-sample of 8 or 16.

- Files with a LIST-INFO chunk aren't accepted. You can use `ffmpeg -i bad.wav -bitexact good.wav` for a file with a better header.
