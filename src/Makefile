
SRC=$(wildcard *.c)
OBJS=$(SRC:%.c=%.o)
DEPS=$(SRC:%.c=%.d)

MAKEARGS='CFLAGS=${CFLAGS}' 'CC=${CC}' 'LDFLAGS=${LDFLAGS}' \
	'INCLUDEDIR=${INCLUDEDIR}' 'EXECUTABLE=${EXECUTABLE}' \
	'SRC=${SRC}' 'OBJS=${OBJS}' 'DEPS=${DEPS}' 'INC=${INC}'\
	'MEFLAG=${MEFLAG}'

all: build

include ${INC}

# the '-' sign tells Make to not display an error if the include file doesn't 
# exist.  Since .d files are generated they may not exist during some runs of 
# the Makefile.
-include $(SRC:%.c=%.d)

build: 	$(DEPS) $(OBJS)
	#include $(SRC:%.c=%.d)
	@ echo "Linking object files"
	@ $(CC) $(CFLAGS) $(CIFLAGS) $(OBJS) $(LDFLAGS) $(LIFLAGS) -o $(EXECUTABLE)
        @ echo "Completed: $(EXECUTABLE)"

%.o: %.c
	@ echo "Building $@"
	@ $(CC) $(CFLAGS) $(CIFLAGS) $(MEFLAG) -I$(INCLUDEDIR) -o $@ -c $(@:%.o=%.c)

%.d: %.c
	@ echo Generating $@
	@( $(CC) -MM $(CPPFLAGS) -I $(INCLUDEDIR)  $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$ )
