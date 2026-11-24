# TPE Protocolos de Comunicación 2026/2 - proxy SOCKSv5
#
# Targets:
#   make / make all    compila el servidor (bin/socks5d)
#   make test          compila y corre los tests unitarios (requiere libcheck)
#   make clean         borra los artefactos generados
#
# Opciones:
#   make SANITIZE=1    compila con AddressSanitizer + UBSan

CC       := gcc
CPPFLAGS := -D_POSIX_C_SOURCE=200112L -Isrc/utils -Isrc/server -Isrc/server/shared -Isrc/server/proxy -Isrc/server/management
CFLAGS   := -std=c11 -pedantic -pedantic-errors -Wall -Wextra -g -pthread -MMD -MP
LDFLAGS  := -pthread
LDLIBS   :=

ifeq ($(SANITIZE),1)
CFLAGS  += -fsanitize=address,undefined -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address,undefined
endif

SRC_DIR  := src
OBJ_DIR  := obj
BIN_DIR  := bin

# sources

UTILS_SRC  := $(wildcard $(SRC_DIR)/utils/*.c)
SERVER_SRC := $(wildcard $(SRC_DIR)/server/*.c) \
              $(wildcard $(SRC_DIR)/server/shared/*.c) \
              $(wildcard $(SRC_DIR)/server/proxy/*.c) \
              $(wildcard $(SRC_DIR)/server/management/*.c)

UTILS_OBJ  := $(UTILS_SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
SERVER_OBJ := $(SERVER_SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

SERVER_BIN := $(BIN_DIR)/socks5d

# targets

.PHONY: all server test clean

all: server

server: $(SERVER_BIN)

$(SERVER_BIN): $(SERVER_OBJ) $(UTILS_OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	@mkdir -p $@

#  tests

TEST_DIR   := test
TEST_BIN   := $(BIN_DIR)/test
CHECK_LIBS := $(shell pkg-config --libs check 2>/dev/null || echo -lcheck -lm -lrt)
U          := $(SRC_DIR)/utils

TESTS := buffer_test netutils_test parser_test parser_utils_test selector_test stm_test

$(TEST_BIN)/buffer_test:       $(TEST_DIR)/buffer_test.c
$(TEST_BIN)/netutils_test:     $(TEST_DIR)/netutils_test.c     $(U)/netutils.c $(U)/buffer.c
$(TEST_BIN)/parser_test:       $(TEST_DIR)/parser_test.c       $(U)/parser.c
$(TEST_BIN)/parser_utils_test: $(TEST_DIR)/parser_utils_test.c $(U)/parser_utils.c $(U)/parser.c
$(TEST_BIN)/selector_test:     $(TEST_DIR)/selector_test.c
$(TEST_BIN)/stm_test:          $(TEST_DIR)/stm_test.c          $(U)/stm.c

$(TEST_BIN)/%:
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) -I$(TEST_DIR) $(filter-out -MMD -MP,$(CFLAGS)) $^ -o $@ $(LDFLAGS) $(CHECK_LIBS)

test: $(addprefix $(TEST_BIN)/,$(TESTS))
	@set -e; for t in $^; do echo "== $$t"; ./$$t; done

# misc

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

-include $(SERVER_OBJ:.o=.d) $(UTILS_OBJ:.o=.d)
