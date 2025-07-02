CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS = -pthread -lm
SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = tests
BIN_DIR = bin
DATA_DIR = data
LIB_DIR = lib_recommend

# Fichiers sources principaux
SOURCES = $(SRC_DIR)/data_reader.c $(SRC_DIR)/server.c $(SRC_DIR)/client.c
OBJECTS = $(SOURCES:.c=.o)

# Fichiers sources pour la bibliothèque
LIB_SOURCES = $(SRC_DIR)/knn.c $(SRC_DIR)/matrix_factorisation.c $(SRC_DIR)/page_rank_recommendation.c
LIB_OBJECTS = $(LIB_SOURCES:.c=.o)

# Bibliothèques statique et dynamique
STATIC_LIB = $(LIB_DIR)/librecommendation_static.a
DYNAMIC_LIB = $(LIB_DIR)/librecommendation_dynamic.so

# Exécutables
TEST_DATA_READER = $(BIN_DIR)/test_data_reader
TEST_SERVER_CLIENT = $(BIN_DIR)/test_server_client
CLIENT_EXEC = $(BIN_DIR)/client

# Dépendances des headers
DEPS = $(INCLUDE_DIR)/client.h $(INCLUDE_DIR)/server.h $(INCLUDE_DIR)/data_reader.h $(INCLUDE_DIR)/knn.h $(INCLUDE_DIR)/matrix_factorisation.h $(INCLUDE_DIR)/page_rank_recommendation.h

# Vérification du type de liaison (statique ou dynamique)
LINK_TYPE ?= $(shell read -p "Choisir le type de liaison (static/dynamic) [static]: " link_type; echo $${link_type:-static})

ifeq ($(LINK_TYPE),static)
    LIB = $(STATIC_LIB)
    LIB_FLAGS = $(STATIC_LIB)
else ifeq ($(LINK_TYPE),dynamic)
    LIB = $(DYNAMIC_LIB)
    LIB_FLAGS = -L$(LIB_DIR) -lrecommendation_dynamic
else
    $(error LINK_TYPE doit être 'static' ou 'dynamic')
endif

.PHONY: all clean test lib

all: lib $(TEST_DATA_READER) $(TEST_SERVER_CLIENT) $(CLIENT_EXEC)

# Création des bibliothèques
lib: $(LIB)

# Bibliothèque statique
$(STATIC_LIB): $(LIB_OBJECTS)
	@mkdir -p $(LIB_DIR)
	ar rcs $@ $^
	@echo "Bibliothèque statique créée : $@"

# Bibliothèque dynamique
$(DYNAMIC_LIB): $(LIB_OBJECTS)
	@mkdir -p $(LIB_DIR)
	$(CC) -shared -o $@ $(LIB_OBJECTS)
	@echo "Bibliothèque dynamique créée : $@"

# Compilation de l'exécutable de test pour data_reader
$(TEST_DATA_READER): $(TEST_DIR)/test_data_reader.c $(SRC_DIR)/data_reader.o $(LIB)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_data_reader.c $(SRC_DIR)/data_reader.o $(LIB_FLAGS) -o $@ $(LDFLAGS)
	@echo "Exécutable de test data_reader créé : $@"

# Compilation de l'exécutable de test pour server-client
$(TEST_SERVER_CLIENT): $(TEST_DIR)/test_server_client.c $(SRC_DIR)/data_reader.o $(SRC_DIR)/server.o $(LIB)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_server_client.c $(SRC_DIR)/data_reader.o $(SRC_DIR)/server.o $(LIB_FLAGS) -o $@ $(LDFLAGS)
	@echo "Exécutable de test server-client créé : $@"

# Compilation du client
$(CLIENT_EXEC): $(SRC_DIR)/data_reader.o $(SRC_DIR)/client.o $(LIB)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/data_reader.o $(SRC_DIR)/client.o $(LIB_FLAGS) -o $@ $(LDFLAGS)
	@echo "Client créé : $@"

# Règle générale pour compiler les fichiers objets
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@ $(if $(findstring dynamic,$(LINK_TYPE)),-fPIC)

# Exécution des tests
test: $(TEST_DATA_READER) $(TEST_SERVER_CLIENT)
	@if [ "$(LINK_TYPE)" = "dynamic" ]; then \
		export LD_LIBRARY_PATH=$(LIB_DIR):$$LD_LIBRARY_PATH; \
		echo "LD_LIBRARY_PATH défini : $$LD_LIBRARY_PATH"; \
	fi
	./$(TEST_DATA_READER)
	./$(TEST_SERVER_CLIENT)

# Nettoyage
clean:
	rm -rf $(BIN_DIR)/* $(SRC_DIR)/*.o $(LIB_DIR)/*.a $(LIB_DIR)/*.so
	@echo "Nettoyage terminé"

# Aide
help:
	@echo "Cibles disponibles :"
	@echo "  all     - Compile tout (bibliothèque + exécutables, selon LINK_TYPE)"
	@echo "  lib     - Compile la bibliothèque (statique ou dynamique, selon LINK_TYPE)"
	@echo "  test    - Compile et exécute les tests"
	@echo "  clean   - Nettoie les fichiers générés"
	@echo "  help    - Affiche cette aide"
	@echo ""
	@echo "Utilisation :"
	@echo "  make LINK_TYPE=static  - Compile avec la bibliothèque statique"
	@echo "  make LINK_TYPE=dynamic - Compile avec la bibliothèque dynamique"
	@echo "  make                   - Demande à l'utilisateur de choisir static/dynamic"
