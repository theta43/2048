BIN = 2048
VERSION = 1.0
CC = cc

SRC = ${BIN}.c tile.c
OBJS = ${SRC:.c=.o}

INCS = -I../include
CPPFLAGS = -DVERSION=\"${VERSION}\" -D_GNU_SOURCE ${INCS}

DEBUG = -Og -g -DDEBUG
CFLAGS = -std=c99 -Wpedantic -Wall -Wextra -Werror -Os ${DEBUG}

LIBS = -lpthread
LDFLAGS = -s ${LIBS}

all: ${BIN}

${BIN}: ${OBJS}
	${CC} -o ${BIN} ${OBJS} ${LDFLAGS}

.c.o:
	${CC} ${CPPFLAGS} ${CFLAGS} -c $< -o $@

clean:
	-rm -f ${BIN} ${OBJS}

.PHONY: all clean
