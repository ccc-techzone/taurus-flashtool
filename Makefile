CC=clang
CC_FLAGS=-std=c11 -g -Iinclude
LDFLAGS=-lcurses -lmenu -lpanel -lserialport -g

SOURCES=main.c gui.c port.c ops.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE=flashtool

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(CC_FLAGS) $(LDFLAGS) $(OBJECTS) -o $(EXECUTABLE)

%.o: %.c
	$(CC) -c $(CC_FLAGS) $< -o $@

clean:
	rm $(OBJECTS) $(EXECUTABLE)
