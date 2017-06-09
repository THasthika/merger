ACLOCAL_AMFLAGS = -I m4 --install

AM_CFLAGS = --std=c99 -g3 -DNDEBUG -I./libal/src
AM_LDFLAGS = -L./libal/src

bin_PROGRAMS = merger
merger_SOURCES = src/main.c src/merger.h src/merger.c
merger_LDADD = -lal

EXTRA_DIST = m4/NOTES
