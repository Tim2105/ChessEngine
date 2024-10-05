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
CFLAGS = -Wall -Wextra -Werror -std=c++20 -Isrc -Ofast -flto=auto -march=native -DUSE_HCE -DTUNE

# Linkerflags
LDFLAGS = 

# Externe Bibliotheken
LDLIBS = 

# Ausgabe der verwendeten Compilerflags
# (nur wenn das Ziel nicht clean ist)
ifneq ($(MAKECMDGOALS),clean)
$(info Compiling with $(CC) $(CFLAGS))
endif

# Quelldateien
SRC = $(call rwildcard,src/,*.cpp)

# Einzubettende Ressourcen
# Überprüfe, ob -DUSE_HCE gesetzt ist
ifneq ($(findstring USE_HCE,$(CFLAGS)),USE_HCE)
RES = $(call rwildcard,resources/,*.nnue)
else
RES = $(call rwildcard,resources/,*.hce)
endif

# Objektdateien
# Werden in den Ordner bin/obj erstellt
OBJ = $(SRC:src/%.cpp=bin/obj/%.o) $(RES:resources/%=bin/embed/%.o)

# Ausführbare Datei
# Wird in den Ordner bin erstellt
EXEC = bin/main

# Erstellt die Ausführbare Datei
$(EXEC): $(OBJ)
	$(info Linking with $(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS))
	@$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@ $^

# mkdir -p für Windows
MKDIR = $(if $(filter $(OS),Windows_NT),if not exist $(subst /,\,$1) mkdir $(subst /,\,$1),mkdir -p $1)

# Erstellt die Objektdateien aus den Quelldateien
# Fehlende Ordner werden erstellt
bin/obj/%.o: src/%.cpp
	$(info Compiling $<)
	@$(call MKDIR,$(dir $@))
	@$(CC) $(CFLAGS) -c -o $@ $<

# Erstellt die Objektdateien aus den Ressourcen
# Fehlende Ordner werden erstellt
bin/embed/%.o: resources/%
	$(info Embedding $<)
	@$(call MKDIR,$(dir $@))
	@$(LD) -r -b binary -o $@ $<

# Löscht alle erstellten Dateien
clean:
ifeq ($(OS),Windows_NT)
	rmdir /s /q bin
else
	rm -rf bin
endif


# Führt die Ausführbare Datei aus
run: $(EXEC)
	@$(EXEC)