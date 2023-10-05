# Eine Makefile, die Dateien aus dem Verzeichnis "src" kompiliert und
# das Ergebnis in das Verzeichnis "bin" legt.

# Shell für die Ausführung von Befehlen setzen
ifeq ($(OS),Windows_NT)
	SHELL = cmd.exe
else
	SHELL = /bin/bash
endif

# Rekursives Wildcard
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

# Compiler
CC = g++

# Compilerflags
CFLAGS = -Wall -Wextra -Werror -std=c++20 -Ofast -Isrc -flto=auto -mavx2

# Linkerflags
LDFLAGS = 

# Externe Bibliotheken
LDLIBS = 

$(info Compiling with $(CC) $(CFLAGS))

# Quelldateien
SRC = $(call rwildcard,src/,*.cpp)

# Objektdateien
# Werden in den Ordner bin/obj erstellt
OBJ = $(SRC:src/%.cpp=bin/obj/%.o)

# Ausführbare Datei
# Wird in den Ordner bin erstellt
EXEC = bin/main

# Erstellt die Ausführbare Datei
$(EXEC): $(OBJ)
	$(info Linking with $(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS))
	@$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# mkdir -p für Windows
MKDIR = $(if $(filter $(OS),Windows_NT),if not exist $(subst /,\,$1) mkdir $(subst /,\,$1),mkdir -p $1)

# Erstellt die Objektdateien
# Fehlende Ordner werden erstellt
bin/obj/%.o: src/%.cpp
	$(info Compiling $<)
	@$(call MKDIR,$(dir $@))
	@$(CC) $(CFLAGS) -c -o $@ $<

# Löscht alle erstellten Dateien
clean:
ifeq ($(OS),Windows_NT)
	@rmdir /s /q bin
else
	@rm -rf bin
endif


# Führt die Ausführbare Datei aus
run: $(EXEC)
	@$(EXEC)