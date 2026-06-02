PROG = proj

VPATH = src:src/game:src/devices:src/ui:../lab2

SRCS = proj.c \
       game.c \
       player.c \
       keyboard.c \
       mouse.c \
       rtc.c \
       game_render.c \
       menu.c \
       assets.c \
       assets_fields.c \
       utils.c \
       timer.c

CFLAGS += -pedantic -I./src -I../lab2 -I../lab3 -I../lab4 -I../lab5

DPADD += ../lab2/libtimer.a ../lab3/libkbc.a \
         ../lab4/libmouse.a ../lab5/libvideo.a ${LIBLCF}

LDADD += -L../lab2 -ltimer
LDADD += -L../lab3 -lkbc
LDADD += -L../lab4 -lmouse
LDADD += -L../lab5 -lvideo
LDADD += -llcf

.include <minix.lcom.mk>