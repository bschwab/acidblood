
INSTALLER=modinst
MODNAME=dicemod

include ${INC}

build: modules

modules:
	@ echo "Building Module: ${MODNAME}"
	$(CC) $(CFLAGS) $(MFLAGS) -I$(INCLUDEDIR) -c dicemod.c 
	$(MODLINK) $(MODNAME)

clean:
	@ rm -f $(MODNAME).so $(MODNAME).o

distclean: clean
	@ rm -f *~ *.old *.bak enabled ${INSTALLER}

install:
	@sh ${INSTALLER}
