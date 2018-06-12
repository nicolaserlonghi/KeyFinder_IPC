PROJECT    := key_finder
SRC_DIR    := src/
OBJ_DIR    := build/
BIN_DIR    := bin/
SRC        := $(wildcard $(SRC_DIR)*.c)
OBJ        := $(SRC:$(SRC_DIR)%.c=$(OBJ_DIR)%.o)
LD         := gcc
LDLIBS     := -lpthread
CFLAGS     := -I include/
MKDIR      := mkdir -p
EXECUTE	:= boo



ifeq ($(EXECUTE), thread)
$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	@echo Compile $< -> $@
	@$(MKDIR) build
	$(CC) -D THREAD=1 $(CFLAGS) -c $< -o $@ -ggdb

$(PROJECT): $(OBJ)
	@echo Linking $(PROJECT)
	$(LD) $(LDFLAGS) $(OBJ) -o $(PROJECT) -lpthread -ggdb
else
$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	@echo Compile $< -> $@
	@$(MKDIR) build
	$(CC) -D THREAD=0 $(CFLAGS) -c $< -o $@ -ggdb

$(PROJECT): $(OBJ)
	@echo Linking $(PROJECT)
	$(LD) $(LDFLAGS) $(OBJ) -o $(PROJECT) -ggdb
endif

all: $(PROJECT)

install:
	$(MKDIR) $(BIN_DIR)
	cp $(PROJECT) $(BIN_DIR)

doc:
	@echo "Generating doc"
	doxygen doxygen.cfg

threads:
	make EXECUTE=thread
	@echo "Generating with threads"
	

help:
	@echo all: compila tutti i target
	@echo clean: pulisce i file intermedi e l’eseguibile
	@echo doc: genera la documentazione
	@echo help: stampa l’elenco dei target possibili
	@echo install: copia l’eseguibile nella cartella​ bin
	@echo threads:​ compila la versione con le threads

clean:
	@echo Clean
	rm $(OBJ) $(PROJECT)

.PHONY: all clean install help doc threads