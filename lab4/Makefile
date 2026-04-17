# name of the program (Minix service)
PROG=lab4 

# source code files to be compiled 
SRCS = lab4.c mouse.c

# additional compilation flags
CFLAGS += -pedantic
CPPFLAGS += -I../lab2 -I../lab3

# list of library dependencies
DPADD += ${LIBLCF}
LDADD += -llcf -L../lab2 -ltimer -L../lab3 -lkbc

# include LCOM's makefile that does all the "heavy lifting"
.include <minix.lcom.mk>