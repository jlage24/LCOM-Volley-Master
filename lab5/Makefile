# name of the program (Minix service)
PROG=lab5 

# source code files to be compiled 
.PATH.c: ../lab2
SRCS = lab5.c utils.c

# additional compilation flags
CFLAGS += -pedantic
CPPFLAGS += -I../lab2 -I../lab3 -I../lab4

# list of library dependencies
DPADD += ${LIBLCF} ${LIB_NAME}
LDADD += -llcf -L. -lvideo -L../lab2 -ltimer -L../lab3 -lkbc 

# video library config
LIB_NAME = libvideo.a
LIB_SRCS = video.c
LIB_OBJS = ${LIB_SRCS:.c=.o}
CLEANFILES += ${LIB_NAME} ${LIB_OBJS}

all: lib
lib: ${LIB_OBJS}
	ar rcs ${LIB_NAME} ${LIB_OBJS}

# include LCOM's makefile that does all the "heavy lifting"
.include <minix.lcom.mk>