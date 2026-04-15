CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L

VERSION    := $(shell cat VERSION 2>/dev/null || echo "0.1.0")
GIT_COMMIT := $(shell git rev-parse --short HEAD 2>/dev/null || echo "unknown")
BUILD_DATE := $(shell date -u +"%Y-%m-%dT%H:%M:%SZ")

# Use pkg-config for dependencies
PKGS = wayland-client cairo pango pangocairo libinput libudev xkbcommon gtk+-3.0 appindicator3-0.1
# Add -I. to find generated headers in root
CFLAGS += -I. $(shell pkg-config --cflags $(PKGS))
LIBS = $(shell pkg-config --libs $(PKGS)) -lm
SRC = src/main.c src/input.c src/shm.c src/buffer.c src/keys.c src/draw.c src/wl_setup.c src/window.c src/tray.c xdg-shell-protocol.c
OBJ = $(SRC:.c=.o)
TARGET = keypop

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

src/version.h: VERSION
	@echo "/* auto-generated, do not edit */"       > $@
	@echo "#pragma once"                            >> $@
	@echo "#define APP_VERSION \"$(VERSION)\""      >> $@
	@echo "#define GIT_COMMIT  \"$(GIT_COMMIT)\""   >> $@
	@echo "#define BUILD_DATE  \"$(BUILD_DATE)\""   >> $@

# Generate protocol code
xdg-shell-protocol.c:
	wayland-scanner private-code /nix/store/lj7cvr4npgvsy8klcga2k85jm2qkvzmy-wayland-protocols-1.47/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml $@

xdg-shell-client-protocol.h:
	wayland-scanner client-header /nix/store/lj7cvr4npgvsy8klcga2k85jm2qkvzmy-wayland-protocols-1.47/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml $@

# Dependencies
src/main.o: src/main.c src/state.h src/wl_setup.h src/window.h src/keys.h src/draw.h src/tray.h xdg-shell-client-protocol.h src/version.h
src/input.o: src/input.c src/input.h
src/shm.o: src/shm.c src/shm.h
src/buffer.o: src/buffer.c src/buffer.h src/state.h
src/keys.o: src/keys.c src/keys.h src/buffer.h src/state.h
src/draw.o: src/draw.c src/draw.h src/shm.h src/state.h
src/wl_setup.o: src/wl_setup.c src/wl_setup.h src/state.h
src/window.o: src/window.c src/window.h src/draw.h src/state.h
src/tray.o: src/tray.c src/tray.h src/state.h src/window.h

clean:
	rm -f src/*.o src/version.h xdg-shell-protocol.o $(TARGET) xdg-shell-protocol.c xdg-shell-client-protocol.h

install: $(TARGET)
	install -D -m 755 $(TARGET) /usr/local/bin/$(TARGET)

.PHONY: all clean install version bump-patch bump-minor bump-major

version:
	@echo "$(TARGET) v$(VERSION) ($(GIT_COMMIT))"

bump-patch:
	@parts=$$(cat VERSION | tr '.' ' '); \
	major=$$(echo $$parts | cut -d' ' -f1); \
	minor=$$(echo $$parts | cut -d' ' -f2); \
	patch=$$(echo $$parts | cut -d' ' -f3); \
	echo "$$major.$$minor.$$((patch+1))" > VERSION; \
	echo "Bumped to $$(cat VERSION)"

bump-minor:
	@parts=$$(cat VERSION | tr '.' ' '); \
	major=$$(echo $$parts | cut -d' ' -f1); \
	minor=$$(echo $$parts | cut -d' ' -f2); \
	echo "$$major.$$((minor+1)).0" > VERSION; \
	echo "Bumped to $$(cat VERSION)"

bump-major:
	@major=$$(cat VERSION | cut -d'.' -f1); \
	echo "$$((major+1)).0.0" > VERSION; \
	echo "Bumped to $$(cat VERSION)"
