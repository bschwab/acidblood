
MAKEARGS='CFLAGS=${CFLAGS}' 'CC=${CC}' 'LDFLAGS=${LDFLAGS}' \
	'INCLUDEDIR=${INCLUDEDIR}' 'INC=${INC}'

build:
	@for i in * ; do \
		if [ -e $$i/enabled ] ; then \
			( cd $$i ; ${MAKE} -s ${MAKEARGS} modules) ; \
		fi ;\
	done 

install:
	@for i in * ; do \
		if [ -e $$i/enabled ] ; then \
			( cd $$i ; ${MAKE} -s ${MAKEARGS} install) ; \
		fi ;\
	done 

clean:
	@for i in * ; do \
		if [ -e $$i/Makefile ] ; then \
			echo "Cleaning $$i";\
			( cd $$i ; ${MAKE} clean) ; \
		fi ;\
	done 

distclean:
	@for i in * ; do \
		if [ -e $$i/Makefile ] ; then \
			echo "distcleaning $$i";\
			( cd $$i ; ${MAKE} distclean) ; \
		fi \
	done 
