CC ?= clang
CFLAGS ?= -O2 -Wall -Wextra
PREFIX ?= /data/data/com.termux/files/usr
BINDIR ?= $(PREFIX)/bin

TARGET = nowinch

all: $(TARGET)

$(TARGET): nowinch.c
	$(CC) $(CFLAGS) nowinch.c -o $(TARGET)

install: $(TARGET)
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)/$(TARGET)

uninstall:
	rm -f $(BINDIR)/$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all install uninstall clean
