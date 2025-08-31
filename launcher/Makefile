
BUILD_DIR := ../bin
LAUNC_SRC := launch.c
LAUNC_OUT := launch.exe
LAUNC_ARC := -m32
LAUNC_FLG := Wl,-rpath=.
LAUNC_INC := ../loader
LAUNC_LIB := $(BUILD_DIR)
LOADR_NAM := _simpleloader

default:
	gcc $(LAUNC_ARC) -o $(BUILD_DIR)/$(LAUNC_OUT) $(LAUNC_SRC) -I$(LAUNC_INC) -L$(LAUNC_LIB) -l$(LOADR_NAM) $(LAUNC_FLG)
clean:
	rm -f $(BUILD_DIR)/$(LAUNC_OUT)