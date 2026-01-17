SHELL=/bin/bash

COV := 1

CXX := g++
LD := g++
CXXFLAGS := -O0 -g --std=c++20
LDFLAGS := -Llibs/zlib -lz  #-Wl,--verbose
ifeq ($(COV),1)
	CXXFLAGS := $(CXXFLAGS) -fprofile-arcs -ftest-coverage
	LDFLAGS := $(LDFLAGS) -lgcov --coverage
endif

SRC_DIR := src
INC_DIR := src/include
OBJ_DIR := obj
BUILD_DIR := build
SCRIPTS_DIR := scripts
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
HEADERS := $(wildcard $(INC_DIR)/*.hpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
DEPENDENCIES := -Ilibs/zlib
BINARY := $(BUILD_DIR)/patchit

VERSION := $(shell ./$(SCRIPTS_DIR)/getversion.sh)
COMPATIBILITY_VERSION := 0

TESTS_DIR := tests
TESTS_LOGS_DIR := logs
TESTS_RUNTIME_DIR := runtime_testing
TESTS_RUNTIME_DATA := runtime_data

UNIT_TESTS_DIR := unit
UNIT_TESTS_OBJ := obj/unit
UNIT_TESTS_SOURCES = $(wildcard $(UNIT_TESTS_DIR)/*.cpp)
UNIT_TESTS_HEADERS = $(wildcard $(UNIT_TESTS_DIR)/*.hpp)
UNIT_TESTS_OBJECTS := $(patsubst $(UNIT_TESTS_DIR)/%.cpp,$(UNIT_TESTS_OBJ)/%.o,$(UNIT_TESTS_SOURCES))
UNIT_TESTS_DEP_OBJS := $(filter-out $(OBJ_DIR)/main.o,$(OBJECTS))
UNIT_TESTS_BINARY := $(BUILD_DIR)/unit

$(BINARY): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)

$(OBJECTS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c -o $@ $< -I${INC_DIR} ${DEPENDENCIES} \
		-DPATCHIT_VERSION='"$(VERSION)"' \
		-DPATCHIT_COMPATIBILITY_VERSION=$(COMPATIBILITY_VERSION)

test: $(BINARY)
	bash $(TESTS_DIR)/runtests.sh $(BINARY)

$(UNIT_TESTS_BINARY): $(UNIT_TESTS_OBJECTS) $(UNIT_TESTS_DEP_OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

$(UNIT_TESTS_OBJECTS): $(UNIT_TESTS_OBJ)/%.o: $(UNIT_TESTS_DIR)/%.cpp $(HEADERS) $(UNIT_TESTS_HEADERS)
	$(CXX) $(CXXFLAGS) -c -o $@ $< -I${INC_DIR} -I${UNIT_TESTS_DIR} \
		-DPATCHIT_VERSION='"$(VERSION)"' \
		-DPATCHIT_COMPATIBILITY_VERSION=$(COMPATIBILITY_VERSION)

unit: $(UNIT_TESTS_BINARY)
	@echo =====================================================
	./$(UNIT_TESTS_BINARY)
	@echo =====================================================

.PHONY: cov
cov:
	lcov --capture --directory $(OBJ_DIR) --output-file coverage.info
	genhtml coverage.info --output-directory cov

.PHONY: unitcov
unitcov:
	make clean || true
	make unit || true
	make cov || true

.PHONY: init
init:
	mkdir -p build obj obj/unit

.PHONY: libs
libs:
	make -C libs/zlib

.PHONY: clean
clean:
	rm -rf $(wildcard ${BUILD_DIR}/*) \
        $(wildcard ${OBJ_DIR}/*) \
		$(TESTS_RUNTIME_DIR) \
		$(TESTS_LOGS_DIR) \
		$(TESTS_RUNTIME_DATA) \
		cov
	make init

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
	@echo UNIT_TESTS_DEP_OBJS = $(UNIT_TESTS_DEP_OBJS)

.PHONY: format
format:
	bash $(SCRIPTS_DIR)/format

