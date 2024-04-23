ifndef debug
debug=1 # Default mode
endif

RM=rm
CLI_VERSION=1.0.0

BUILD_DIR=build
BUILD_DIR_TESTS=$(BUILD_DIR)/tests
SRC_DIR=src
TEST_DIR=test
SANDBOX_DIR=sandbox
# INSTALL_PREFIX = /usr
# INSTALL_PREFIX_BIN = /usr/bin

CC= gcc
INCS= -I. -I./lib
# LIBS = -l:libcurl.so.4 -l:libjson-c.so.5 -l:libpackcomp.so.1
LIBS = -l:libglfw.so.3 -l:libvulkan.so.1
TEST_LIBS = -l:libcriterion.so.3
# LIBS = -lcurl -ljson-c

DEBFLAGS = -g -Wall -fsanitize=address # -Wpedantic 
RELFLAGS = -O2
MODE_FLAGS = $(RELFLAGS)
ifeq ($(strip $(debug)),1)
MODE_FLAGS = $(DEBFLAGS)
endif

CPPFLAGS = -DVERSION=\"${CLI_VERSION}\"
CFLAGS = -std=c2x $(MODE_FLAGS) $(INCS)
LDFLAGS = $(LIBS)
TEST_LDFLAGS = $(LDFLAGS) $(TEST_LIBS)

# CLI_BIN = packcomp

# CLI_SRC = $(SRC_DIR)/main.cc

# SRCS = $(shell find $(SRC_DIR) -name "*.cc")
# HS = $(shell find $(SRC_DIR) -name "*.cc")
# # SRCS_D1 = $(shell find $(SRC_DIR) -maxdepth 1 -name "*.cc")
# SRCS_D1 = $(SRC_DIR)/packcomp.cc
# TEST_SRCS = $(shell find $(TEST_DIR) -maxdepth 1 -name "*.cc")
# TESTS = $(addprefix $(BUILD_DIR_TESTS)/, $(basename $(notdir $(TEST_SRCS))))

