
#acidblood

CC=gcc
EXECUTABLE=acidblood
SUBDIRS=src
INSTALLER=installer
LOADER=loader
INCLUDEDIR=include
MODPATH=dmodules

# for production
CFLAGS+=-O2 -Wall -march=native

CFLAGS+=-pipe
# for debugging each line, use DDEBUGLINE
#CFLAGS+=-g -DDEBUG -DDEBUGLINE
#CFLAGS+=-g -DDEBUG

# ------------------------------------------------------------------------
# --------------------- Do Not Edit below this line! ---------------------
# ------------------------------------------------------------------------

CFLAGS+=$(shell ./dist/ccflags $(CC)) 

MAKEARGS='CFLAGS=${CFLAGS}' 'CC=${CC}' 'LDFLAGS=${LDFLAGS}' \
	'INCLUDEDIR=$(PWD)/${INCLUDEDIR}' 'EXECUTABLE=${EXECUTABLE}' \
	'INC=$(PWD)/Makefile.inc' 'TOPDIR=$(PWD)'

all: build

build: main

install: main
	@sh ${INSTALLER}
	@( cd ${MODPATH}; ${MAKE} -s ${MAKEARGS} install; )

run: install
	@sh ${LOADER}
	
main: 
	@if [ ! -f include/config.h ] ; then \
		echo "Running Config" ; \
		sh Config ; \
	fi
	@if [ ! -f Makefile.inc ] ; then \
		echo "Running Config" ; \
		sh Config ; \
	fi
	@if [ -f Modules.enabled ] ; then \
		for i in $(SUBDIRS); do \
			( cd $$i; ${MAKE} -s ${MAKEARGS} 'MEFLAG=-DMODULES' build; ) ; \
		done ; \
	else \
		for i in $(SUBDIRS); do \
		( cd $$i; ${MAKE} -s ${MAKEARGS} build; ) ; \
		done ;\
	fi
	@( cd ${MODPATH}; ${MAKE} -s ${MAKEARGS} build; ) ;

clean:
	@ ( rm -f src/${EXECUTABLE}) 
	@for i in $(SUBDIRS); do \
		echo "cleaning $$i";\
		( cd $$i; rm -f *.o *.d ; ) ; \
	done 
	@( cd ${MODPATH}; ${MAKE} clean; ) ;


distclean: clean
	@( rm -f ${INCLUDEDIR}/config.h installer Config.options *~ *.bak *.old)
	@( rm -f ${LOADER} ${INSTALLER} Makefile.inc Modules. Modules.enabled Modules.disabled ) 
	@( rm -f ${INCLUDEDIR}/*~ ${INCLUDEDIR}/*.bak ${INCLUDEDIR}/*.old)
	@( rm -f ${MODPATH}/*~ ${MODPATH}/*.bak ${MODPATH}/*.old)
	@( cd ${MODPATH}; ${MAKE} distclean; )
	@for i in $(SUBDIRS); do \
		echo "distleaning: $$i";\
		( cd $$i; rm -f *~ *.bak *.old ; ) ; \
	done
