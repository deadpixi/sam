# config.mk - makefile configuration for sam
# copyright 2015 Rob King <jking@deadpixi.com>

# DESTDIR is the root of the installation tree
DESTDIR?=/usr/local

# BINDIR is the directory where binaries go
BINDIR?=$(DESTDIR)/bin

# MANDIR is where manual pages go
MANDIR?=$(DESTDIR)/share/man/man1

# USE64BITS should be 1 for little-endian architectures with 64-bit pointers,
# 2 for big-endian architectures with 64-bit pointers, and 0 otherwise.
# x86_64 systems would generally use 1 here, while DEC Alpha systems would
# generally use 2.
USE64BITS?=1

# FREETYPEINC should name the directory of your freetype2 includes.
FREETYPEINC?=/usr/include/freetype2

# TMPDIR should be set to a directory for temporary files with lots of room
TMPDIR?=/tmp

# If you want to have keyboard control of dot's position, set the following
# variables to the appropriate ASCII control codes. The default values
# emulate the WordStar diamond. Setting any command to zero disables it.
LINEUP=0x05
LINEDOWN=0x18
CHARLEFT=0x13
CHARRIGHT=0x04