CFLAGS = -Wall -g  
LDFLAGS =  

OBJS = main.o
INFO = diskinfo.o
LIST = disklist.o
GET = diskget.o
PUT = diskput.o
UTIL = fileutils.c


all: diskinfo disklist diskget diskput

diskinfo: $(INFO) $(UTIL)
	$(CC) $(CFLAGS) -o diskinfo $(INFO) $(LDFLAGS) -lreadline -lhistory -ltermcap

disklist: $(LIST) $(UTIL)
	$(CC) $(CFLAGS) -o disklist $(LIST) $(LDFLAGS) -lreadline -lhistory -ltermcap

diskget: $(GET) $(UTIL)
	$(CC) $(CFLAGS) -o diskget $(GET) $(LDFLAGS) -lreadline -lhistory -ltermcap

diskput: $(PUT) $(UTIL)
	$(CC) $(CFLAGS) -o diskput $(PUT) $(LDFLAGS) -lreadline -lhistory -ltermcap

clean: 
	rm -rf $(OBJS) $(INFO) $(LIST) $(GET) $(PUT) diskinfo disklist diskget diskput
