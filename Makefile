NAME        := ft_ping
BUILD_DIR   := build
BUILD_TYPE  ?= Release
CMAKE       := cmake

ifeq ($(OS),Windows_NT)
EXE_EXT := .exe
else
EXE_EXT :=
endif

BINARY := $(BUILD_DIR)/bin/$(NAME)$(EXE_EXT)

.PHONY: all clean fclean re debug run

all:
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	$(CMAKE) --build $(BUILD_DIR) --config $(BUILD_TYPE) --parallel

clean:
	-$(CMAKE) --build "$(BUILD_DIR)" --target clean --config "$(BUILD_TYPE)"

fclean:
	$(CMAKE) -E rm -rf "$(BUILD_DIR)"

re: fclean all

debug:
	$(MAKE) BUILD_TYPE=Debug all

run: all
	"$(BINARY)"
