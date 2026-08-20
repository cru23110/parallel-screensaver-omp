CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
SDL_FLAGS := $(shell pkg-config --cflags --libs sdl2 2>/dev/null)
OMP_FLAGS := -fopenmp

BIN_DIR := bin

.PHONY: all secuencial paralelo clean

all: secuencial paralelo

secuencial: $(BIN_DIR)/screensaver_seq

paralelo: $(BIN_DIR)/screensaver_par

$(BIN_DIR)/screensaver_seq: src/secuencial/main.cpp include/screensaver.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@ $(SDL_FLAGS)

$(BIN_DIR)/screensaver_par: src/paralelo/main.cpp include/screensaver.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OMP_FLAGS) $< -o $@ $(SDL_FLAGS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)
