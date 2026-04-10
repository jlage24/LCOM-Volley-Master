# name of the program (Minix service)
PROG=lab3 

# source code files to be compiled 
SRCS = lab3.c

# additional compilation flags
CFLAGS += -pedantic -DLAB3
CPPFLAGS += -I../lab2

# list of library dependencies
DPADD += ${LIBLCF} ${LIB_NAME}
LDADD += -llcf -L. -lkbc        
LDADD += -L../lab2 -ltimer      

# Keyboard's library configs
LIB_NAME = libkbc.a             
LIB_SRCS = kbc.c                
LIB_OBJS = ${LIB_SRCS:.c=.o}
CLEANFILES += ${LIB_NAME} ${LIB_OBJS}

all: lib
lib: ${LIB_OBJS}
	ar rcs ${LIB_NAME} ${LIB_OBJS}

# include LCOM's makefile that does all the "heavy lifting"
.include <minix.lcom.mk>