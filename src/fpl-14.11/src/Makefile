# Linux Makefile v2 August 21 1996 (thanx to Bjorn Reese)

########################################################################
#                                                                      #
# fpl.library - A shared library interpreting script langauge.         #
# Copyright (C) 1992-1996 FrexxWare                                    #
# Author: Daniel Stenberg                                              #
#                                                                      #
# This program is free software; you may redistribute for non          #
# commercial purposes only. Commercial programs must have a written    #
# permission from the author to use FPL. FPL is *NOT* public domain!   #
# Any provided source code is only for reference and for assurance     #
# that users should be able to compile FPL on any operating system     #
# he/she wants to use it in!                                           #
#                                                                      #
# You may not change, resource, patch files or in any way reverse      #
# engineer anything in the FPL package.                                #
#                                                                      #
# This program is distributed in the hope that it will be useful,      #
# but WITHOUT ANY WARRANTY; without even the implied warranty of       #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                 #
#                                                                      #
# Daniel Stenberg                                                      #
# Ankdammsgatan 36, 4tr                                                #
# S-171 43 Solna                                                       #
# Sweden                                                               #
#                                                                      #
# FidoNet 2:201/328    email:dast@sth.frontec.se                       #
#                                                                      #
########################################################################

.SUFFIXES: .o .c .c~ .h .h~ .a .i

################################################
# Below follows all UNIX macros/actions/lines: #
################################################
TARGET  = FPL
OBJS = script.o numexpr.o hash.o statement.o memory.o frontend.o \
          reference.o sprintf.o scan.o compile.o sscanf.o

# if using the `gcc' compiler:
CFLAGS  = -g -DUNIX  -ansi -pedantic -O -funsigned-char
CC      = gcc
LD      = gcc
LDFLAGS = 

#$(TARGET) : $(OBJS)
#	$(CC) $(OBJS) -o $(TARGET)

$(TARGET) : $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o

$(OBJS): script.h FPL.h

script.o:script.c
numexpr.o:numexpr.c
statement.o:statement.c
hash.o:hash.c
memory.o:memory.c memory.h
frontend.o:frontend.c
reference.o:reference.c
sprintf.o:sprintf.c
scan.o:scan.c
