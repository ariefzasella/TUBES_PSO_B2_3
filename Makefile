CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g
TARGET  = myshell
OBJS    = main.o history.o alias.o parser.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

main.o:    main.c history.h alias.h parser.h
history.o: history.c history.h
alias.o:   alias.c alias.h
parser.o:  parser.c parser.h

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all run clean
