CC=gcc-10
CH=cppcheck
VG=valgrind
STD=c11
TIMEOUT=2s
DEBUG=YES
COMPACT=YES
ALL=YES

DASH=./dash.c
INC=./include/
TEST=./test/test.dsh
FILES=${addprefix ${INC},${shell ls ${INC}}} ${DASH}
REPORT=cppcheck.report
#SUPRLIST= --suppressions-list=./supr.list

JOBS=4
LOAD=4
DEPTH=20
REDIR= --output-file=${REPORT}

loopkill = timeout ${TIMEOUT} $(1) || ( echo; echo Убито.)

ifeq (${DEBUG},YES)
	OPT=-O0 -ggdb
else
	OPT=-O3
endif

ifeq (${COMPACT},YES)
	CHTEMP={file}:{line}| {message} | [CWE:{cwe}]
else
	CHTEMP=CWE{cwe}: {message}\n{callstack}\n{code}
endif

ifeq (${ALL},YES)
	ENABLE= --enable=all --inconclusive --bug-hunting
else
	ENABLE= --enable=warning
endif

GCCFLAGS=${OPT} -std=${STD} -Wall
CHFLAGS=-I${INC} --language=c -j${JOBS} -l${LOAD} --max-ctu-depth=${DEPTH} --std=${STD} ${SUPRLIST} --template='${CHTEMP}' --cppcheck-build-dir=./ ${ENABLE} ${REDIR}
VGFLAGS= --leak-check=full --show-leak-kinds=all --track-origins=yes -s

dash: ${FILES} makefile
	${CC} -I ${INC} ${GCCFLAGS} ${DASH} -o $@

check:
	${CH} ${CHFLAGS} ${FILES} | grep %
	@cat ${REPORT} | grep CWE | wc -l | (read n; echo Обнаружено $$n ошибок.)

test:
	${call loopkill,${DASH:.c=} ${TEST}}
	
vg:
	${call loopkill,${VG} ${VGFLAGS} ${DASH:.c=} ${TEST}}

report:
	cat ${REPORT} | column -t -s '|'

git:
	@if [ ${shell ${DASH:.c=} ${TEST} | grep ERROR | wc -l} -ne 0 ]; \
	then \
		echo; echo Не все тесты пройдены, отказ.; \
	else \
		${MAKE} clean; \
		git status;    \
		read a;        \
		git add .;     \
		git commit -S && git push;      \
	fi
stat:
	@echo СТАТИСТИКА: ${shell cat ${FILES} | wc -l} строк.
.PHONY: check report test vg git clean
clean:
	rm -f ${DASH:.c=}
	rm -f *.a1
	rm -f *.txt
	rm -f ${REPORT}
