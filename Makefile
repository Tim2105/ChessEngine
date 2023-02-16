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
CFLAGS = -Wall -Wextra -Werror -std=c++20 -O3 -Isrc

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
	$(CC) $(CFLAGS) -o $@ $^

# Erstellt die Objektdateien
# Fehlende Ordner werden erstellt
bin/obj/%.o: src/%.cpp
	$(info Compiling $<)
	@if not exist "$(subst /,\,$(@D))" mkdir $(subst /,\,$(@D))
	@$(CC) $(CFLAGS) -c -o $@ $<


# Löscht alle erstellten Dateien
clean:
	@rmdir /s /q bin


# Führt die Ausführbare Datei aus
run: $(EXEC)
	@$(EXEC)