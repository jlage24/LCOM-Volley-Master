# name of the program (Minix service)
PROG=lab3 

# source code files to be compiled
SRCS = lab3.c kbc.c

# additional compilation flags
CFLAGS += -pedantic -DLAB3

# list of library dependencies
DPADD += ${LIBLCF}
LDADD += -llcf -L../lab2 -ltimer

# include LCOM's makefile that does all the "heavy lifting"
.include <minix.lcom.mk>