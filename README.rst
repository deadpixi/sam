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

*Note that this is not stable software.*
This version of sam is under heavy development.
That being said, it's my primary editor, so any major bugs tend to get fixed pretty quickly.
Still, be careful with it.

The Obiligatory Screenshot
--------------------------

.. image:: sam.png

Community
=========

Rob posts updates about sam on Twitter at http://twitter.com/TheKingAdRob.

There's a mailing list and discussion group available at https://groups.google.com/forum/#!forum/deadpixi-sam.
The list is low-volume and used mostly for announcements and discussion about features.

Installation
============

Basic Installation
-------------------
Installation and configuration is fairly simple:

- Copy `config.mk.def` to `config.mk` and modify as needed.
- Copy `config.h.def` to `config.h` and modify as needed.
- Copy `commands.h.def` to `commands.h` and modify as needed.
- Copy `chords.h.def` to `chords.h` and modify as needed.
- Run `make clean all`
- Run `make install` or, if needed, `sudo make install`

Note that running `make install` will install a `desktop entry file`_, in either "system" or "user" mode.
This can be specified via the `MODE` make variable (the default is "user").
To isntall the desktop entry for the all users, use:

    make MODE=system install

The `sam` command runs sam.
The `B` command adds a new file to a running instance of sam, or starts sam if it's not already running.

.. _`desktop entry file`: https://specifications.freedesktop.org/desktop-entry-spec/latest/

Running Remotely
--------------------

Both the `sam` and `B` commands accept an '-r' argument,
naming a remote machine or
(assuming you're using `ssh(1)`, an SSH host entry).
The remote machine needs to have both `sam` and `rsam` installed.

The remote machine may also have the `B` command installed.
If it is installed,
the `B` command can be executed both locally (on the machine running `samterm`) using the '-r' option,
and remotely (on the machine running `sam`) without the '-r' option.

Installation Paths
-------------------

By default, `sam`, `rsam`, `samterm`, and `B` all end up in '$(BINDIR)' as defined in config.mk.

Compatibility
-------------

Note that Deadpixi sam has extended the binary protocol spoken between sam and samterm.
That means that,
in its default configuration,
a Deadpixi samterm won't work with a non-Deadpixi sam
nor will a Deadpixi sam work with a non-Deadpixi samterm.

Defining `CLASSIC_SAM_COMPATIBILITY` in `config.h` will allow backwards-compatibility between Deadpixi and class sam,
but at the expense of some of the newer features.

(And note that there may come a time where there is a hard break with the past!)

New Features
============

Modern OS Support
    This version of sam compiles and runs on modern Linux/Unix/BSD systems.

Improved 64-bit Support
    The original sam had support for 32-bit architectures and big-endian 64-bit architectures.
    This version supports 64-bit architectures of any endianess (including, most importantly, x86_64).

Scalable Font Support
    This version of sam is not limited to classic X fonts, but can use modern scalable fonts.
    Inconsolata_ is this author's favorite, though `Courier Prime Code`_ is giving Inconsolata a run for its money.

Multicolor Support
    This version of sam supports colors, including different background colors for different files.
    This allows different files to be easily distinguished.
    The default is still the classic two-color appearance, of course.

Simplified and Dynamic Configuration
    The graphical elements (colors and fonts) of this version of sam are controlled via environment variables, not X Resources.
    The table of composable characters is now dynamically configurable (via the `~/.keyboard` file),
    where it was once hard-coded.

Far Better Keyboard Support
    The selection ("cursor") can be moved using keyboard commands.
    Additional keyboard-accessible commands allow jumping between file windows and the command window,
    scrolling the display, snarfing, pasting, etc.
    The binding of these commands to keyboard sequences is configurable at compile-time.

Support for Two-Button Mice and Wheel Mice
    The original sam required a three-button mouse.
    This version is still easier to use with such a mouse, but can be used with a two-button mouse by simulating a button-3 press using shift-button-2.
    This version also supports scrolling with mouse wheels.

Support for Mouse Chords
    The commands available for keyboard binding are also accessible via mouse-button combinations ("chords").
    By default, the snarf, cut, and paste commands are mapped to chords.
    The binding of these chords is configurable at compile-time.

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

The Future
==========

This project has an end goal:
once the issues listed here are complete,
this edition of sam will enter maintenance mode.

Primary Goals
-------------

- Scalable font support (DONE)
- Support big- and little-endian 64-bit systems (DONE)
- Support compilation and use on modern \*nix systems (DONE)
- Runtime configuration of composition sequences (DONE)
- Support two-button mice (DONE)
- Support tab expansion (DONE)
- Support runtime configuration of tab sizes (DONE)
- Support scroll wheels on mice (DONE)
- Support fuzzy matching in the `b` command (DONE)
- Raise the window when opening a new file (DONE)
- Support a configurable set of keybindings (i.e. rework the keyboard layer) (DONE)
- Support multiple background colors at once (DONE)
- Support the following commands for keybindings
    - escape (DONE)
    - scrollup / scrolldown (DONE)
    - charright / charleft (DONE)
    - lineup / linedown (DONE)
    - jump to/from command window (DONE)
    - delword / delbol / del (DONE)
    - snarf / cut / paste / exchange (DONE)
    - write (DONE)
    - nextlayer / prevlayer (TODO)
    - maximize / tile left / tile right (TODO)
    - look (TODO)
    - /regex (TODO)
    - send (TODO)
    - eol / bol (DONE)
- Support a configurable scroll factor;
  scrolling is a bit drastic now (TODO)
- Support Unicode beyond the Basic Multilingual Plane
  (note that this will break the sam binary protocol,
  so this version of samterm won't work with other sams!
  Email me if you think this is a bad idea; I'm willing to reconsider) (TODO)
- Support font fallback (TODO)
- Allow runtime configuration of key bindings (TODO)
- Support a configurable set of mouse chords (DONE)
- Support runtime configuration of mouse chords (TODO)
- Support mouse button reassignment (TODO)
- Support runtime mouse button reassignment (TODO)
- Remove non-*nix OS support (Plan 9 has their own sam) (TODO)
- Remove external command FIFO, switch to X ClientMessage messages for IPC
  (email me if you want to know why I think this is a good idea) (TODO)
- Support the CDPATH environment variable for the `cd` command (TODO)
- Split the man page into documentation for `samterm`, `sam`, `keyboard`, and `samrc`
  (if and when `samrc` becomes a thing) (TODO)
- Add localization support (TODO)
- Add a Desktop Entry file, icon, etc (TODO)
- Create RPMs, DEBs, etc (TODO)
- Refactor all code to be as clean and standards-compliant as possible;
  remove all legacy code (TODO)
- Compile with no warnings,
  with all warnings and `-pedantic` enabled on GCC in C99 mode (TODO)

Stretch Goals
-------------
- Remove Xt dependency (TODO)
- Switch to a more X11-y model (e.g. one child window per layer) (TODO)

Very Unlikely Goals
-------------------
- Windows port (no, seriously, stop laughing)
- Non-X11 Mac OS X port
- Console port

Permissible Changes in Maintenance Mode
---------------------------------------
Once the above goals are met, the only changes that will be made to sam are:

- Bugfixes
- Translation updates
- Binary package updates
- Updates necessary to keep sam compiling on whatever systems its users are using

Things That Won't Ever Happen (Sorry)
-------------------------------------
- Syntax highlighting
- Multiple cursors
- Complex text rendering
  (I really am sorry about this one;
  I want speakers of languages with more complex writing systems to use sam,
  but getting it to work would be nigh impossible)

How You Can Help
================

- Use sam!
  Open up issues on GitHub if you see any problems or have any ideas.
- Spread sam!
  Tell your friends and colleagues.
  Anyone know Rob Pike, Brian Kernighan, Ken Thompson, or Bjarne Stroustrup?
  They are known sam-users, see if they like this version. :)
- Package sam!
  Create packages or ports of sam for your operating system of choice.
- Translate sam!
  Currently, sam only speaks English.
  I'd like to see sam speak all of the languages that its users speak.
- Write sam!
  Write code and send patches.

Credits
=======

These credits are in rough chronological order:

Rob Pike, Howard Trickey, Matty Farrow, Boyd Roberts, Doug Gwyn, James Clark, Mark H. Wilkinson, et al.
    Authors and/or contributors to the original X version of sam,
    upon which this version is based.

Rob King
    Added most of the things mentioned in `New Features`_ above.
    Rob is the author and maintainer of this version of sam.

Chris Siebenmann
    Fixed various bugs in font rendering, and ported Mark H. Wilkinson's mouse chording code to this version of sam.
    Provided initial implementations of the Cscroll{up,down}line commands.

Aram Hăvărneanu
    Improved the handling of Makefile variables.

Ishpeck
    Improved C89 support.

Tommy Pettersson
    Fixed bugs in the cursor movement code.

Christian Neukirchen
    Fixed various Makefile bugs.

Benjamin Scher Purcell
    Added the Cbol and Ceol commands.

If I've forgotten you in this list of credits, please accept my apologies and email me (Rob King) at jking@deadpixi.com to be added.

Copyright and License
=====================

The authors of this software are Rob Pike and Howard Trickey.
Copyright (c) 1998 by Lucent Technologies.

Rob King made some changes.
Those changes, Copyright (c) 2014-2016 by Rob King.

Permission to use, copy, modify, and distribute this software for any
purpose without fee is hereby granted, provided that this entire notice
is included in all copies of any software which is or includes a copy
or modification of this software and in all copies of the supporting
documentation for such software.

THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR LUCENT TECHNOLOGIES MAKE ANY
REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
