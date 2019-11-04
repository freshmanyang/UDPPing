################################################
#
#  Makefile
#
#  To see the value of any variable, issue:
#   make print-variable
#     Ex, to see LIBS :   make print-LIBS
#
#  Make.defines brings in
#    OSFLAG LIBS GPSLIB LINKOPTIONS
#    OPTIONS  WARNING
#    CFLAGS CPLUSFLAGS  CLEANFILES
#   As well as the CC, C, GCC, ... bindings
#
##############################################
include ./commonCode/Make.defines

VPATH = .:./commonCode


PROGS =	  UDPPingServer UDPPingClient  GetAddrInfo testAddress


COBJECTS =	AddressHelper.o utils.o SocketHelper.o timeHelper.o delayHelper.o messages.o
CSOURCES =	AddressHelper.c utils.c SocketHelper.c timeHelper.c delayHelper.c messages.c

CLEANFILES =     UDPPingServer.o UDPPingClient.o GetAddrInfo.o testAddress.o


CPLUSOBJECTS =

COMMONSOURCES =

CPLUSSOURCES =

SOURCES =       ${CSOURCES} ${COMMONSOURCES} ${CPLUSSOURCES}
OBJECTS =       ${COBJECTS} ${COMMONOBJECTS} ${CPLUSOBJECTS}

LINKLIBS = ${LIBS} ${GPSLIB}



#The following rule is handy...
#to see value of make variables, such as AllObjects:
#issue 'make print-PROGS' 
print-%:
	@echo '$*=$($*)'

all:	${PROGS}


GetAddrInfo:	GetAddrInfo.c GetAddrInfo.o $(OBJECTS) $(SOURCES)
		${CC} ${LINKOPTIONS}  $@ GetAddrInfo.o $(OBJECTS) $(LINKLIBS) 

testAddress:	testAddress.c testAddress.o $(OBJECTS) $(SOURCES)
		${CC} ${LINKOPTIONS}  $@ testAddress.o $(OBJECTS) $(LINKLIBS) 


UDPPingClient:	UDPPingClient.o $(CPLUSOBJECTS) $(COBJECTS) $(LIBS) $(COMMONSOURCES) $(SOURCES)
		${CC} ${LINKOPTIONS}  $@ UDPPingClient.o $(CPLUSOBJECTS) $(COBJECTS) $(BASELIBS) $(LIBS) $(LINKFLAGS)

UDPPingServer:	UDPPingServer.o $(CPLUSOBJECTS) $(COBJECTS) $(LIBS) $(COMMONSOURCES) $(SOURCES)
		${CC} ${LINKOPTIONS}  $@ UDPPingServer.o $(CPLUSOBJECTS) $(COBJECTS) $(BASELIBS) $(LIBS) $(LINKFLAGS)



.cc.o:	$(HEADERS)
	$(CPLUS) $(CPLUSFLAGS) $(OPTIONS) $<

.c.o:	$(HEADERS)
	$(CC) $(CFLAGS) $(OPTIONS) $<

# make print-VARIABLE
#  make print-LIBS
#  make print-LINKLIBS
print-%  : ; @echo $* = $($*)




#Note the tar derefences sym links - this is ok for creating a backup for submission. But
#Not ok for creating a backup and then trying to restore for use.
backup:
	rm -f UDPPing.tar.gz
	rm -f UDPPing.tar
	tar -hcf UDPPing.tar *
	gzip -f UDPPing.tar


clean:
		rm -f ${PROGS} ${COBJECTS} ${CPLUSOBJECTS} ${CLEANFILES}


