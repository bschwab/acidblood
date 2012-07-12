
INSTALLER=modinst
MODNAME=seen

include ${INC}

build: modules

modules:
	@ echo "Building Module: ${MODNAME}"
	$(CC) $(CFLAGS) $(MFLAGS) -I$(INCLUDEDIR) -c ${MODNAME}.c 
	$(MODLINK) $(MODNAME)

clean:
	@ rm -f $(MODNAME).so $(MODNAME).o

distclean: clean
	@ rm -f *~ *.old *.bak enabled ${INSTALLER}

install:
	@sh ${INSTALLER}
