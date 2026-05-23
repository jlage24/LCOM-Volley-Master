PROG = proj

VPATH = src:src/game:src/input:src/ui:../lab2

SRCS = proj.c player.c keyboard.c game_render.c timer.c

CFLAGS += -pedantic -I../lab2 -I../lab3 -I../lab4 -I../lab5 -I./src
DPADD += ${LIBLCF} ../lab2/libtimer.a ../lab3/libkbc.a \
         ../lab4/libmouse.a ../lab5/libvideo.a
LDADD += -llcf
LDADD += -L../lab2 -ltimer
LDADD += -L../lab3 -lkbc
LDADD += -L../lab4 -lmouse
LDADD += -L../lab5 -lvideo

.include <minix.lcom.mk>