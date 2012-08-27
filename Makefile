#Simple Makefile

VPATH 		+= src

vpath 		%.h 	src/ui
vpath 		%.c 	src/ui

CC 		:= gcc
CFLAGS 		:= -Wall -Werror -std=gnu99 -Isrc
CFLAGS 		+= -I /usr/include/libxml2/
CFLAGS 		+= $(shell pkg-config --cflags --libs gtk+-2.0)
CFLAGS 		+= $(shell imlib2-config --cflags --libs)

LDLIBS 		+= -L /usr/lib -lmpdclient
LDLIBS 		+= -L /usr/lib -lxml2
LDLIBS 		+= -L /usr/lib -lpcre

MAIN 		:= ./src/main.c
BINNAME 	:= mpdcoverart

SOURCE 		:= $(filter-out $(MAIN), $(shell find ./src -type f -name "*.c"))
HEAD 		:= -I src/ui
OBJECT 		:= $(patsubst %.c, %.o, $(SOURCE))
#OBJECT 		:= $(subst %.c,%o, $(SOURCE))
#OBJECT 		:= $(foreach n, $(SOURCE), $(basename $(n)).o)


.PHONY: all clean cscope

all: $(OBJECT) main

main: $(OBJECT)
	$(CC) $(CFLAGS) $(LDLIBS) $(HEAD) $(OBJECT) $(MAIN) -o $(BINNAME)

#Generator all obj file
$(STATIC): $(OBJECT)
#$(OBJECT): %.o: %.c

cscope:
	find ./src -name "*.c" -o -name "*.h" > cscope.files
	cscope -Rbq

clean:
	find -name "*.o" -exec rm {} \;
	-rm test
	-rm $(BINNAME)

install: main
	mkdir -p /usr/share/mpdcoverart
	cp -rv "Magnifique Bright" /usr/share/mpdcoverart/
	mkdir -p ~/.config/mpdcoverart
	cp -rv mpdcoverart.conf ~/.config/mpdcoverart/
	cp $(BINNAME) /usr/bin/

uninstall:
	rm -rfv /usr/share/mpdcoverart/
	rm -rfv ~/.config/mpdcoverart/
	rm /usr/bin/$(BINNAME)
