# Eine Makefile, die Dateien aus dem Verzeichnis "src" kompiliert und
# das Ergebnis in das Verzeichnis "bin" legt.

# Compiler
CC = g++

# Compilerflags
CFLAGS = -Wall -Wextra -Werror -std=c++20 -O3 -Isrc

# Quelldateien
SRC = $(shell find src -name "*.cpp")

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
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<