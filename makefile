CC = gcc
CFLAGS = -O3 -std=c99 -Wall -Wformat -Wunused-variable
LDFLAGS =
INSTALL = /usr/bin/install -c

PREFIX = /usr/local
BIN = $(PREFIX)/bin
MAN = $(PREFIX)/share/man/man1

SOURCES = $(wildcard ./src/*.c)

all:
	@echo '========================================================='
	@echo 'You must run make with a target corresponding to your OS.'
	@echo 'Options: linux, solaris, bsd, osx, win.'
	@echo '========================================================='

linux: x11test gltest
	$(eval SOURCES+=$(wildcard ./src/plat/linux/*.c))
	$(eval CFLAGS+='-D_POSIX_C_SOURCE=200112L' -Wno-unused-result)
	$(eval LDFLAGS+=-lpthread -lX11 -lGL)
	$(CC) $(CFLAGS) $(SOURCES) -o ./screenfetch-c $(LDFLAGS)

solaris: x11test
	$(eval SOURCES+=./src/plat/sun/*.c)
	$(eval CFLAGS+='-D_POSIX_C_SOURCE=200112L')
	$(eval LDFLAGS+=-lpthread -lX11)
	$(CC) $(CFLAGS) $(SOURCES) -o ./screenfetch-c $(LDFLAGS)

bsd:
	$(eval SOURCES+=./src/plat/bsd/*.c)
	$(eval CFLAGS+='-D_POSIX_C_SOURCE=200112L')
	$(eval LDFLAGS+=-lpthread)
	$(CC) $(CFLAGS) $(SOURCES) -o ./screenfetch-c $(LDFLAGS)

osx:
	$(eval SOURCES+=./src/plat/darwin/*.c)
	$(eval CFLAGS+='-D_POSIX_C_SOURCE=200112L')
	$(eval LDFLAGS+=-lpthread -framework CoreServices)
	$(CC) $(CFLAGS) $(SOURCES) -o ./screenfetch-c $(LDFLAGS)

win:
	$(eval SOURCES+=./src/plat/win32/*.c)
	$(eval CFLAGS+=-DWIN32_LEAN_AND_MEAN)
	$(eval CFLAGS+='-D_POSIX_C_SOURCE=200112L')
	$(CC) $(CFLAGS) $(SOURCES) -o ./screenfetch-c $(LDFLAGS)

install:
	$(INSTALL) screenfetch-c $(BIN)/screenfetch-c
	$(INSTALL) ./src/scripts/detectde $(BIN)/detectde
	$(INSTALL) ./src/scripts/detectgtk $(BIN)/detectgtk
	$(INSTALL) ./src/scripts/detectwm $(BIN)/detectwm
	$(INSTALL) ./src/scripts/detectwmtheme $(BIN)/detectwmtheme
	$(INSTALL) ./src/scripts/detectgpu $(BIN)/detectgpu
	mkdir -p $(MAN)
	$(INSTALL) ./man/screenfetch-c.1 $(MAN)/screenfetch-c.1

uninstall:
	rm -rf $(BIN)/screenfetch-c
	rm -rf $(BIN)/detectde
	rm -rf $(BIN)/detectgtk
	rm -rf $(BIN)/detectwm
	rm -rf $(BIN)/detectwmtheme
	rm -rf $(BIN)/detectgpu
	rm -rf $(MAN)/screenfetch-c.1

x11test:
	@echo "Testing for X11..."
	$(CC) $(CFLAGS) ./src/tests/x11test.c -o ./x11test -lX11
	rm -f ./x11test

gltest:
	@echo "Testing for OpenGL..."
	$(CC) $(CFLAGS) ./src/tests/gltest.c -o ./gltest -lGL
	rm -f ./gltest

threadtest:
	$(CC) $(CFLAGS) ./src/thread.c ./src/tests/threadtest.c -o ./threadtest -lpthread
	./threadtest
	rm -f ./threadtest

clean:
	rm -f threadtest
	rm -f x11test
	rm -f gltest
	rm -f screenfetch-c

