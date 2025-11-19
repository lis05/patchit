SHELL=/bin/bash

CXX := g++
LD := g++
CXXFLAGS := -O0 -g --std=c++20
LDFLAGS :=

SRC_DIR := src
INC_DIR := src/include
OBJ_DIR := obj
BUILD_DIR := build
SCRIPTS_DIR := scripts
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
HEADERS := $(wildcard $(INC_DIR)/*.hpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
BINARY := $(BUILD_DIR)/patchit

VERSION := $(shell ./$(SCRIPTS_DIR)/getversion.sh)
COMPATIBILITY_VERSION := 0

TESTS_DIR := tests
TESTS_LOGS_DIR := testing_logs
TESTS_RUNTIME_DIR := runtime_testing

$(BINARY): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJECTS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c -o $@ $< -I${INC_DIR} \
		-DPATCHIT_VERSION='"$(VERSION)"' \
		-DPATCHIT_COMPATIBILITY_VERSION=$(COMPATIBILITY_VERSION)

test: $(OBJECTS)
	bash $(TESTS_DIR)/runtests.sh $(BINARY)


.PHONY: clean
clean:
	rm -rf $(wildcard ${BUILD_DIR}/*) \
        $(wildcard ${OBJ_DIR}/*) \
		$(TESTS_RUNTIME_DIR) \
		$(TESTS_LOGS_DIR)

.PHONY: vars
vars:
	@echo CXX = $(CXX)
	@echo LD = $(CXX)
	@echo CXXFLAGS = $(CXXFLAGS)
	@echo LDFLAGS = $(LDFLAGS)
	@echo SRC_DIR = $(SRC_DIR)
	@echo INC_DIR = $(INC_DIR)
	@echo OBJ_DIR = $(OBJ_DIR)
	@echo BUILD_DIR = $(BUILD_DIR)
	@echo SCRIPTS_DIR = $(SCRIPTS_DIR)
	@echo SOURCES = $(SOURCES)
	@echo HEADERS = $(HEADERS)
	@echo OBJECTS = $(OBJECTS)
	@echo BINARY = $(BINARY)
	@echo VERSION = $(VERSION)
	@echo COMPATIBILITY_VERSION = $(COMPATIBILITY_VERSION)

.PHONY: format
format:
	bash $(SCRIPTS_DIR)/format

