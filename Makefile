# Eine Makefile, die Dateien aus dem Verzeichnis "src" kompiliert und
# das Ergebnis in das Verzeichnis "bin" legt.

ifeq ($(OS),Windows_NT)
    SHELL = cmd.exe
else
    SHELL = /bin/bash
endif

# Parallele Ausführung
ifeq ($(OS),Windows_NT)
  NUMCPUS := $(NUMBER_OF_PROCESSORS)
else
  NUMCPUS := $(shell nproc)
endif

MAKEFLAGS += --no-print-directory -j$(NUMCPUS)

# Standardziel
.DEFAULT_GOAL := engines

# Rekursives Wildcard
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

# Compiler
CC = g++
LD = ld

# Compilerflags (Header Dependency Tracking hinzugefügt: -MMD -MP)
DEPFLAGS = -MMD -MP
CFLAGS_BASE = -Wall -Wextra -Werror -std=c++20 -Isrc -Ofast -flto=auto -march=native $(DEPFLAGS) -DNDEBUG
CFLAGS_HCE = $(CFLAGS_BASE) -DUSE_HCE
CFLAGS_NNUE = $(CFLAGS_BASE) -DUSE_NNUE
CFLAGS_REN = $(CFLAGS_BASE) -DUSE_REN

# Profiling-Varianten
CFLAGS_GEN = -fprofile-generate
CFLAGS_USE = -fprofile-use -fprofile-correction

# Linkerflags
ifeq ($(OS),Windows_NT)
LDFLAGS = 
else
LDFLAGS = -z noexecstack
endif

LDLIBS =

# Argumente für den Profiling-Prozess
PROFILING_ARGS = "go movetime 3000"

# Ausgabe der verwendeten Compilerflags
ifneq ($(MAKECMDGOALS),clean)
$(info [INFO] Compiling with $(CC) $(CFLAGS_BASE))
endif

# Quelldateien
SRC = $(call rwildcard,src/,*.cpp)

# Ressourcen
RES_HCE = $(call rwildcard,resources/,*.hce)
RES_NNUE = $(call rwildcard,resources/,*.nnue)
RES_REN = $(call rwildcard,resources/,*.ren)

# Gemeinsame Quelldateien
SRC_ENGINE = $(filter-out src/tune/%.cpp src/emscripten/%.cpp,$(SRC))

# Engine-Objekte
ENGINE_OBJ_NNUE = $(patsubst src/%.cpp,bin/obj_nnue/%.o,$(SRC_ENGINE)) $(patsubst resources/%,bin/embed_nnue/%.o,$(RES_NNUE))
ENGINE_OBJ_HCE = $(patsubst src/%.cpp,bin/obj_hce/%.o,$(SRC_ENGINE)) $(patsubst resources/%,bin/embed_hce/%.o,$(RES_HCE))

# Engine-Ziele
ENGINE_NNUE = bin/nnue_engine
ENGINE_HCE = bin/hce_engine

# Tuning-Objekte
SRC_TUNE_HCE = $(filter-out src/emscripten/%.cpp src/main.cpp src/tune/nnue/%.cpp src/tune/ren/%.cpp,$(SRC))
TUNE_HCE_OBJ = $(patsubst src/%.cpp,bin/obj_hce/%.o,$(SRC_TUNE_HCE)) $(patsubst resources/%,bin/embed_hce/%.o,$(RES_HCE))
TUNE_HCE = bin/hce_tune

SRC_TUNE_NNUE = $(filter-out src/emscripten/%.cpp src/main.cpp src/tune/hce/%.cpp src/tune/ren/%.cpp,$(SRC))
TUNE_NNUE_NNUE_OBJ = $(patsubst src/%.cpp,bin/obj_nnue/%.o,$(SRC_TUNE_NNUE)) $(patsubst resources/%,bin/embed_nnue/%.o,$(RES_NNUE))
TUNE_NNUE = bin/nnue_tune

SRC_TUNE_REN = $(filter-out src/emscripten/%.cpp src/main.cpp src/tune/nnue/%.cpp src/tune/hce/%.cpp,$(SRC))
TUNE_REN_OBJ = $(patsubst src/%.cpp,bin/obj_ren/%.o,$(SRC_TUNE_REN)) $(patsubst resources/%,bin/embed_ren/%.o,$(RES_REN))
TUNE_REN = bin/ren_tune

release: clean
	@$(MAKE) profile-gen
	@$(MAKE) profile-use

.PHONY: all clean profile profile-gen profile-use engines clean-profile clean-nonprofile

# Allgemeines Ziel
all: $(ENGINE_NNUE) $(ENGINE_HCE) $(TUNE_HCE) $(TUNE_NNUE) $(TUNE_REN)

# Spezifische Ziele
nnue: $(ENGINE_NNUE) $(TUNE_NNUE)
hce: $(ENGINE_HCE) $(TUNE_HCE)
ren: $(TUNE_REN)

# Nur Engines
engines: $(ENGINE_NNUE) $(ENGINE_HCE)

# Engine ohne USE_HCE
$(ENGINE_NNUE): $(ENGINE_OBJ_NNUE)
	@echo [LINK][NNUE]     Engine: $@
	@$(CC) $(CFLAGS_NNUE) $(LDFLAGS) $(LDLIBS) -o $@ $^

# Engine mit USE_HCE
$(ENGINE_HCE): $(ENGINE_OBJ_HCE)
	@echo [LINK][HCE]     Engine: $@
	@$(CC) $(CFLAGS_HCE) $(LDFLAGS) $(LDLIBS) -o $@ $^

# TUNE_HCE
$(TUNE_HCE): $(TUNE_HCE_OBJ)
	@echo [LINK][HCE]     Tuning: $@
	@$(CC) $(CFLAGS_HCE) $(LDFLAGS) $(LDLIBS) -o $@ $^

# TUNE_NNUE
$(TUNE_NNUE): $(TUNE_NNUE_NNUE_OBJ)
	@echo [LINK][NNUE]     Tuning: $@
	@$(CC) $(CFLAGS_NNUE) $(LDFLAGS) $(LDLIBS) -o $@ $^

# TUNE_REN
$(TUNE_REN): $(TUNE_REN_OBJ)
	@echo [LINK][REN]     Tuning: $@
	@$(CC) $(CFLAGS_REN) $(LDFLAGS) $(LDLIBS) -o $@ $^

# mkdir -p für Windows
MKDIR = $(if $(filter $(OS),Windows_NT),if not exist $(subst /,\,$1) mkdir $(subst /,\,$1),mkdir -p $1)

# Compile-Regeln
bin/obj_nnue/%.o: src/%.cpp
	@echo [CXX][NNUE]     $<
	@$(call MKDIR,$(dir $@))
	@$(CC) $(CFLAGS_NNUE) -c -o $@ $<

bin/obj_hce/%.o: src/%.cpp
	@echo [CXX][HCE]     $<
	@$(call MKDIR,$(dir $@))
	@$(CC) $(CFLAGS_HCE) -c -o $@ $<

bin/obj_ren/%.o: src/%.cpp
	@echo [CXX][REN]     $<
	@$(call MKDIR,$(dir $@))
	@$(CC) $(CFLAGS_REN) -c -o $@ $<

bin/embed_nnue/%.o: resources/%
	@echo [EMBED][NNUE]     $<
	@$(call MKDIR,$(dir $@))
	@$(LD) -r -b binary -o $@ $<

bin/embed_hce/%.o: resources/%
	@echo [EMBED][HCE]     $<
	@$(call MKDIR,$(dir $@))
	@$(LD) -r -b binary -o $@ $<

bin/embed_ren/%.o: resources/%
	@echo [EMBED][REN]     $<
	@$(call MKDIR,$(dir $@))
	@$(LD) -r -b binary -o $@ $<


# Profiling-Ziel: generate
profile-gen: CFLAGS_NNUE += $(CFLAGS_GEN)
profile-gen: CFLAGS_HCE += $(CFLAGS_GEN)
profile-gen: clean-profile engines
	@echo [PROFILE]  Profiling run started, this may take a while...
ifeq ($(OS),Windows_NT)
	@bin\nnue_engine.exe $(PROFILING_ARGS) > nul
	@bin\hce_engine.exe $(PROFILING_ARGS) > nul
else
	@./bin/nnue_engine $(PROFILING_ARGS) > /dev/null
	@./bin/hce_engine $(PROFILING_ARGS) > /dev/null
endif
	@$(MAKE) clean-nonprofile

# Profiling-Ziel: use
profile-use: CFLAGS_NNUE += $(CFLAGS_USE)
profile-use: CFLAGS_HCE += $(CFLAGS_USE)
profile-use: engines
	@$(MAKE) clean-profile

# Clean-Ziele
clean:
ifeq ($(OS),Windows_NT)
	@if EXIST bin ( \
		rmdir /s /q bin \
	)
else
	@rm -rf bin
endif

clean-profile:
ifeq ($(OS),Windows_NT)
	@IF EXIST bin ( \
		FOR /R bin %%F IN (*.gcda) DO DEL /Q "%%F" \
	)
else
	@if [ -d bin ]; then \
		find bin -name '*.gcda' -delete; \
	fi
endif

clean-nonprofile:
ifeq ($(OS),Windows_NT)
	@IF EXIST bin ( \
		FOR /R bin %%F IN (*.o) DO DEL /Q "%%F" \
	)
else
	@if [ -d bin ]; then \
		find bin -name '*.o' -delete; \
	fi
endif

# --- Dependency Inclusion ---
# Finde alle .d Dateien, die vom Compiler generiert wurden und binde sie ein
DEP_FILES = $(patsubst src/%.cpp,bin/obj_nnue/%.d,$(SRC)) \
            $(patsubst src/%.cpp,bin/obj_hce/%.d,$(SRC)) \
			$(patsubst src/%.cpp,bin/obj_ren/%.d,$(SRC)) \
			$(patsubst resources/%,bin/embed_nnue/%.d,$(RES_NNUE)) \
			$(patsubst resources/%,bin/embed_hce/%.d,$(RES_HCE)) \
			$(patsubst resources/%,bin/embed_ren/%.d,$(RES_REN))

-include $(DEP_FILES)