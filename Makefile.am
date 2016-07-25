ACLOCAL_AMFLAGS = -I m4 --install

AM_CFLAGS = --std=c99 -g3 -DNDEBUG

bin_PROGRAMS = merger
merger_SOURCES = src/main.c src/merger.h src/merger.c
merger_LDADD = -lTHB

EXTRA_DIST = m4/NOTES
