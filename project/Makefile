PROG = proj

SRCS = src/main.c src/game/game.c src/game/ball.c \
       src/game/player.c src/game/score.c \
       src/ui/menu.c src/ui/hud.c \
       src/ui/game_render.c \
       src/input/keyboard.c

CFLAGS += -pedantic -I../lab2 -I../lab3 -I../lab4 -I../lab5 -I./src

DPADD += ${LIBLCF} ../lab2/libtimer.a ../lab3/libkbc.a \
         ../lab4/libmouse.a ../lab5/libvideo.a

LDADD += -llcf
LDADD += -L../lab2 -ltimer
LDADD += -L../lab3 -lkbc
LDADD += -L../lab4 -lmouse
LDADD += -L../lab5 -lvideo

.include <minix.lcom.mk>