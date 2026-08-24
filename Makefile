CC = gcc
CFLAGS = -Wall -Wextra -O2 -D_GNU_SOURCE -std=c99
LDFLAGS =

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1

TARGET = uran
SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean distclean install uninstall help test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(TARGET)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)
	install -d $(DESTDIR)$(MANDIR)
	install -m 644 uran.1 $(DESTDIR)$(MANDIR)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(MANDIR)/uran.1

clean:
	rm -f $(OBJECTS) $(TARGET)

distclean: clean
	rm -f *~ *.bak

test: $(TARGET)
	@echo "Running basic tests..."
	@./$(TARGET) -n 5 > /dev/null && echo "...Basic generation"
	@./$(TARGET) -n 3 -m 1 -M 10 > /dev/null && echo "...Range generation"
	@./$(TARGET) -n 2 -x > /dev/null && echo "...Hex output"
	@./$(TARGET) -n 2 -o > /dev/null && echo "...Octal output"
	@./$(TARGET) -n 2 -b > /dev/null && echo "...Binary output"
	@./$(TARGET) -n 1 -N > /dev/null && echo "...Non-blocking mode"
	@./$(TARGET) -n 1 -m 4294967295 -M 4294967295 > /dev/null && echo "...Max range"
	@echo "All tests passed."

help:
	@echo "Available targets:"
	@echo "  all         - Build the utility (default)"
	@echo "  install     - Install binary and man page"
	@echo "  uninstall   - Remove installed files"
	@echo "  clean       - Remove object files and binary"
	@echo "  distclean   - Full cleanup"
	@echo "  test        - Run basic tests"
	@echo "  help        - Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  PREFIX      - Installation prefix (default: /usr/local)"
	@echo "  DESTDIR     - Temporary directory for packaging"
