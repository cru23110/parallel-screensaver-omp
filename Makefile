CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Iinclude
OMPFLAGS := -fopenmp

# Banderas de SDL2. Se detectan solas con pkg-config o con sdl2-config, que es
# lo normal en Linux, macOS y MSYS2. Si tu instalacion no trae ninguno de los
# dos (por ejemplo, SDL2 descomprimido a mano en Windows), pasalas asi:
#
#   make SDL_FLAGS="-IC:/SDL2/include/SDL2 -LC:/SDL2/lib -lmingw32 -lSDL2main -lSDL2"
#
SDL_FLAGS ?= $(shell pkg-config --cflags --libs sdl2 2>/dev/null || sdl2-config --cflags --libs 2>/dev/null)

BIN_DIR := bin

HEADERS    := $(wildcard include/*.hpp)
COMMON_SRC := $(wildcard src/common/*.cpp)
SEQ_SRC    := $(COMMON_SRC) $(wildcard src/secuencial/*.cpp)
PAR_SRC    := $(COMMON_SRC) $(wildcard src/paralelo/*.cpp)

.PHONY: all secuencial paralelo clean

all: secuencial paralelo

secuencial: $(BIN_DIR)/screensaver_seq

paralelo: $(BIN_DIR)/screensaver_par

$(BIN_DIR)/screensaver_seq: $(SEQ_SRC) $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(SEQ_SRC) -o $@ $(SDL_FLAGS)

$(BIN_DIR)/screensaver_par: $(PAR_SRC) $(HEADERS) | $(BIN_DIR)
	@test -f src/paralelo/physics.cpp || { \
	  echo "Falta src/paralelo/physics.cpp: la version paralela se implementa en la Fase 2 (ver PLAN.md)."; \
	  exit 1; }
	$(CXX) $(CXXFLAGS) $(OMPFLAGS) $(PAR_SRC) -o $@ $(SDL_FLAGS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)
