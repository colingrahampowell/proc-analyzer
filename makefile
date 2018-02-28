########
# Colin Powell
# CS373: Defense Against the Dark Arts
# HW5: Process Analyzer
#######

# set compiler, flags
CC = gcc
CCFLAGS = -Wall


SRC1 = proc-analyzer.c
SRC2 = helpers.c
SRCS = ${SRC1} ${SRC2}

PROG1 = proc-analyzer
PROGS = ${PROG1}

all:
	${CC} ${CCFLAGS} ${SRCS} -o ${PROGS}

debug:
	${CC} ${CCFLAGS} -g ${SRCS} -o ${PROGS}

clean:
	rm -rf ${PROGS}
