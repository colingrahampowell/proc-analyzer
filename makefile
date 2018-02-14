########
# Colin Powell
# 
#  Process Analyzer
#######

# set compiler, flags
CC = gcc
CCFLAGS = -Wall


SRC1 = proc-analyzer.c
SRCS = ${SRC1}

PROG1 = proc-analyzer
PROGS = ${PROG1}

all:
	${CC} ${CCFLAGS} ${SRCS} -o ${PROGS}

debug:
	${CC} ${CCFLAGS} -g ${SRCS} -o ${PROGS}

clean:
	rm -rf ${PROGS}
