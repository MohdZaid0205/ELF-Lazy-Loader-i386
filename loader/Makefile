
BUILD_DIR := ../bin
LOADR_SRC := loader.c allocator.c seghandler.c
LOADR_OUT := lib_simpleloader.so
LOADR_ARC := -m32
LOADR_FLG := -fPIC -shared -I./loader

default:
	gcc $(LOADR_FLG) $(LOADR_ARC) -o $(BUILD_DIR)/$(LOADR_OUT) $(LOADR_SRC)
debug:
	gcc $(LOADR_FLG) $(LOADR_ARC) -o $(BUILD_DIR)/$(LOADR_OUT) $(LOADR_SRC) -DLOADER_DEBUG
clean:
	rm -f $(BUILD_DIR)/$(LOADR_OUT)