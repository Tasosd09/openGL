# This sample Makefile allows you to make any SRGP application
#   whose source is exactly one .c file.
#
# To use this Makefile, you must type:
#
#        make PROG=xxxx
#                  ----
# where
#       xxxx.cpp is the name of the file you wish to compile.
#       ----

CC = g++

GCCFLAGS = -fpcc-struct-return
CFLAGS = -g
ifeq ($(OSTYPE),solaris)	
INCLUDE = -I/usr/local/include 
LDFLAGS = -L/usr/local/lib/ 
LDLIBS =  -lGL -lglut -lm 
else
INCLUDE = -I/usr/include
LDFLAGS = -L/usr/lib/
LDLIBS =  -lGLU -lglut -lm
endif


$(PROG):
	$(CC) $(GCCFLAGS) $(INCLUDE) $(CFLAGS) $(PROG).cpp $(LDFLAGS) $(LDLIBS) -o $(PROG)
default:
	example

