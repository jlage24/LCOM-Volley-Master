# name of the program (Minix service)
PROG=lab4 

# source code files to be compiled (só a lógica principal)
SRCS = lab4.c

# additional compilation flags
CFLAGS += -pedantic
CPPFLAGS += -I../lab2 -I../lab3

# list of library dependencies
DPADD += ${LIBLCF} ${LIB_NAME}
LDADD += -llcf -L. -lmouse -L../lab2 -ltimer -L../lab3 -lkbc

# mouse library config
LIB_NAME = libmouse.a
LIB_SRCS = mouse.c
LIB_OBJS = ${LIB_SRCS:.c=.o}
CLEANFILES += ${LIB_NAME} ${LIB_OBJS}

all: lib
lib: ${LIB_OBJS}
	ar rcs ${LIB_NAME} ${LIB_OBJS}

# include LCOM's makefile that does all the "heavy lifting"
.include <minix.lcom.mk>