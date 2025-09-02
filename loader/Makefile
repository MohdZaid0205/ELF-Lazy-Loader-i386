
BUILD_DIR := ../bin
LOADR_SRC := loader.c
LOADR_OUT := lib_simpleloader.so
LOADR_ARC := -m32
LOADR_FLG := -fPIC -shared

default:
	gcc $(LOADR_FLG) $(LOADR_ARC) -o $(BUILD_DIR)/$(LOADR_OUT) $(LOADR_SRC)
clean:
	rm -f $(BUILD_DIR)/$(LOADR_OUT)