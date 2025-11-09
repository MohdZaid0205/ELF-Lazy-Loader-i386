# Assumption about flags of test makefile was made
# makefile of test is supposed to implement all configuration
# it must generate executable properly and into $(TESTS_DIR)		

BUILD_TYP := default
BUILD_DIR := bin
TESTS_TYP := all
TESTS_DIR := test
LOADR_DIR := loader
LAUNC_DIR := launcher

default: prepare compile 
prepare:
    # if bin directory does not exist in cwd.
	@if [ ! -d $(BUILD_DIR) ]; then mkdir $(BUILD_DIR); fi
	@if [ ! -d $(TESTS_DIR) ]; then echo NO TEST FOUND; fi
compile:
    # make all sub directories using default.
	make -C $(LOADR_DIR) $(BUILD_TYP)  
	make -C $(LAUNC_DIR) $(BUILD_TYP)
	make -C $(TESTS_DIR) $(TESTS_TYP)
debug:
	# make all sub directories using default.
	make -C $(LOADR_DIR) debug
	make -C $(LAUNC_DIR) $(BUILD_TYP)
	make -C $(TESTS_DIR) $(TESTS_TYP)
clean:
    # clean all sub directories using clean.
	make -C $(LOADR_DIR) clean
	make -C $(LAUNC_DIR) clean
	make -C $(TESTS_DIR) clean