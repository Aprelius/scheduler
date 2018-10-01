CC = gcc
CMAKE = cmake
CXX = g++
MKDIR = mkdir
RM = rm

ARCH = $(shell uname -m)
OS = $(shell uname -a | awk '{print tolower($$1)}')
CC_NAME = $(shell ${CC} --version | awk '{print $$1}' | head -n1)

BUILD_PATH = build
PROJECT_PATH = $(BUILD_PATH)/projects/$(OS)-$(ARCH)-$(CC_NAME)-
ROOT = $(shell pwd)

CMAKE_OPTIONS = -DCMAKE_CXX_COMPILER=$(CXX) -DCMAKE_C_COMPILER=$(CC)
CMAKE_OPTIONS += -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

default: debug

debug: _premake
	@echo "make[2]: Building debug project folder"
	test -d $(PROJECT_PATH)$@ || $(MKDIR) $(PROJECT_PATH)$@
	@echo "cmake[1]: Starting cmake execution"
	cd $(PROJECT_PATH)$@ && $(CMAKE) -G "Unix Makefiles" $(CMAKE_OPTIONS) \
		  -DCMAKE_BUILD_TYPE="Debug" $(ROOT)
	@echo "cmake[1]: CMake execution complete"
	@echo "make[3]: Building project: $(BUILD_PATH)"
	$(MAKE) -C $(PROJECT_PATH)$@
	@echo "make[3]: Build complete: $(BUILD_PATH)"
	@echo "make[2]: Completed building project"

release: _premake
	@echo "make[2]: Building debug project folder"
	test -d $(PROJECT_PATH)$@ || $(MKDIR) $(PROJECT_PATH)$@
	@echo "cmake[1]: Starting cmake execution"
	cd $(PROJECT_PATH)$@ && $(CMAKE) -G "Unix Makefiles" $(CMAKE_OPTIONS) \
		  -DCMAKE_BUILD_TYPE="Release" $(ROOT)
	@echo "cmake[1]: CMake execution complete"
	@echo "make[2]: Building project: $(BUILD_PATH)"
	$(MAKE) -C $(PROJECT_PATH)$@
	@echo "make[2]: Build complete: $(BUILD_PATH)"
	@echo "make[1]: Completed building project"

clean:
	@echo "make[1]: Cleaning build artifacts"
	test -d $(BUILD_PATH) && $(RM) -rf $(BUILD_PATH) || true
	@echo "make[1]: Clean complete"

_premake:
	@echo "make[1]: Creating build folder"
	test -d $(BUILD_PATH) || $(MKDIR) $(BUILD_PATH)
	test -d $(BUILD_PATH)/projects || $(MKDIR) $(BUILD_PATH)/projects

.PHONY: clean default _premake