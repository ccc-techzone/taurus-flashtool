CC=clang
CC_FLAGS=-std=c11 -g -Iinclude
LDFLAGS=-lncurses -lmenu -lpanel -lserialport

SOURCES=main.c gui.c port.c ops.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE=flashtool

.PHONY: clean dist

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(CC_FLAGS) $(OBJECTS) $(LDFLAGS) -o $(EXECUTABLE)

%.o: %.c
	$(CC) -c $(CC_FLAGS) $< -o $@

clean:
	rm $(OBJECTS) $(EXECUTABLE)

dist:
	cp $(EXECUTABLE) dist/