BIN = 2048
VERSION = 1.0a
CC = cc

SRC = ${BIN}.c tile.c
OBJS = ${SRC:.c=.o}

INCS = -Iinclude
CPPFLAGS = -DVERSION=\"${VERSION}\" -D_POSIX_C_SOURCE=2 ${INCS}

#CFLAGS = -std=c99 -Wpedantic -Wall -Wextra -Werror -g -Og ${CPPFLAGS}
CFLAGS = -std=c99 -Wpedantic -Wall -Wextra -Werror -Os ${CPPFLAGS}

LIBS = -lpthread
LDFLAGS = -s ${LIBS}

all: ${BIN}

${BIN}: ${OBJS}
	${CC} -o ${BIN} ${OBJS} ${LDFLAGS}

.c.o:
	${CC} ${CFLAGS} -c $< -o $@

clean:
	-rm -f ${BIN} ${OBJS}

.PHONY: all clean
