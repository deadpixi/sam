Introduction
============

sam is a text editor originally written for the Blit_ graphical terminal connected to a machine running `9th Edition Research Unix`_.
It contained many useful innovations, the most famous of which was its use of `structural regular expressions`_.

sam was the standard text editor for `Plan 9 from Bell Labs`_, and the Plan 9 version was backported to Unix and the X Window Sytem in the 1980s.

.. _Blit: https://en.wikipedia.org/wiki/Blit_(computer_terminal)

.. _`9th Edition Research Unix`: https://en.wikipedia.org/wiki/Research_Unix

.. _`structural regular expressions`: http://doc.cat-v.org/bell_labs/structural_regexps/se.pdf

.. _`Plan 9 from Bell Labs`: http://plan9.bell-labs.com/plan9/

This version of sam is based on that 1980s Unix port, with many useful additions and modifications (see `New Features`_).

New Features
============

Modern OS Support
    This version of sam compiles and runs on modern Linux/Unix/BSD systems.

Improved 64-bit Support
    The original sam had support 32-bit architectures and big-endian 64-bit architectures.
    This version supports 64-bit architectures of any endianess (including, most importantly, x86_64).

Scalable Font Support
    This version of sam is not limited to classic X fonts, but can use modern scalable fonts.
    Inconsolata_ is this author's favorite, though `Courier Prime Code`_ is giving Inconsolata a run for its money.

Simplified and Dynamic Configuration
    The graphical elements (colors and fonts) of this version of sam are controlled via environment variables, not X Resources.
    The table of composable characters is now dynamically configurable (via the `~/.keyboard` file), where it was once hard-coded.

Far Better Keyboard Support
    The cursor ("selection") can be moved using keyboard commands (a la the `WordStar Diamond`_).
    A keyboard command (ctrl-k) is provided to jump between the command window and the current file window.
    These commands are configurable at compile time (and more commands are in the works).

Support for Two-Button Mice and Wheel Mice.
    The original sam required a three-button mouse.
    This version is still easier to use with such a mouse, but can be used with a two-button mouse by simulating a button-3 press using shift-button-2.
    This version also supports scrolling with mouse wheels.

Support for Mouse Chords
    The snarf, cut, and paste commands are accessible via mouse-button combinations ("chords").

Better Remote Editing Support
    This version of sam can use `ssh(1)` as its remote shell.
    Additionally, the B command works on both the local and the remote system during remote editing sessions.

Command Language Extensions
    Various minor and mostly-compatible changes have been made to the sam command language.
    Most notable is the `b` command, which now performs a fuzzy match on filenames, making switching between files much faster and easier.

Improved Manual Page
    The manual page has been rewritten to use the modern `mdoc(7)` manual page macros.
    It has been additionally cleaned up, clarified, and extended.

Support for Tab Expansion
    When enabled, tabs will be expanded into spaces.

.. _Inconsolata: http://www.levien.com/type/myfonts/inconsolata.html

.. _`Courier Prime Code`: http://quoteunquoteapps.com/courierprime/

.. _`WordStar Diamond`: http://texteditors.org/cgi-bin/wiki.pl?WordStarDiamond

Credits
=======

These credits are in rough chronological order as determined by the ordering in the original README supplied with the X version of sam for those names included there, and then by order of GitHub commits for this version.

Rob Pike
    Original author of sam.

Howard Trickey
    Wrote the X version of the graphics library.

Matty Farrow et al
    Extended the X version of the graphics library to support Unicode.

Boyd Roberts
    Added the external command interface and associated scripts.

Doug Gwyn
    Contributed many useful ideas to the X implementation of sam.

James Clark
    Wrote troff macros to allow the man pages to be rendered on non-V10 Unix systems.

Rob King
    Added most of the things mentioned in `New Features`_ above.
    Rob is the maintainer of this version of sam.

Mark H. Wilkinson
    Wrote the original mouse-chording code.

Chris Siebenmann
    Fixed various bugs in font rendering, and ported Mark H. Wilkinson's mouse chording code to this version of sam.

Aram Hăvărneanu
    Improved the handling of Makefile variables.

Ishpeck
    Improved C89 support.

Tommy Pettersson
    Fixed bugs in the cursor movement code.

Christian Neukirchen
    Fixed various Makefile bugs.

If I've forgotten you in this list of credits, please accept my apologies and email me (Rob King) at jking@deadpixi.com to be added.

Copyright and License
=====================

The authors of this software are Rob Pike and Howard Trickey.
Copyright (c) 1998 by Lucent Technologies.

Rob King made some changes.
Those changes, Copyright (c) 2014-2015 by Rob King.

Permission to use, copy, modify, and distribute this software for any
purpose without fee is hereby granted, provided that this entire notice
is included in all copies of any software which is or includes a copy
or modification of this software and in all copies of the supporting
documentation for such software.

THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR LUCENT TECHNOLOGIES MAKE ANY
REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
