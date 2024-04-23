.SUFFIXES:
MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --no-builtin-variables


include settings.mk

all:

# cli: $(BUILD_DIR)/$(CLI_BIN)

tests.run: lib $(TESTS)
	for test in $(TESTS) ; do \
		LD_LIBRARY_PATH=$(BUILD_DIR) ./$$test ; \
	done

tests: 
ifdef test
	@echo "test: $(test)"
	@make $(BUILD_DIR_TESTS)/$(test)
endif

find:
	find /usr/lib -name "*criterion*"


# install:
# 	mkdir -p $(INSTALL_PREFIX_BIN)
# 	cp -f $(BUILD_DIR)/$(CLI_BIN) $(INSTALL_PREFIX_BIN)
# 	chmod 755 $(INSTALL_PREFIX_BIN)/$(CLI_BIN)



# uninstall:
# 	rm -f $(INSTALL_PREFIX_BIN)/$(CLI_BIN)

clean:
	$(RM) -r $(BUILD_DIR)


$(TEST_DIR)/%.run: $(BUILD_DIR)/$(TEST_DIR)/% 
	$(BUILD_DIR)/$(TEST_DIR)/$*
$(SANDBOX_DIR)/%.run: $(BUILD_DIR)/$(SANDBOX_DIR)/% 
	$(BUILD_DIR)/$(SANDBOX_DIR)/$*

$(SANDBOX_DIR)/%.pp: $(SANDBOX_DIR)/%.c 
	$(CC) -E $(CFLAGS) $(CPPFLAGS) $<

lib/%.check: $(BUILD_DIR)/lib/%.o

$(BUILD_DIR)/lib/%.o: lib/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@ $(LDFLAGS)
	

# $(TEST_DIR)/%: $(BUILD_DIR_TESTS)/% 
	

$(BUILD_DIR)/lib/%: lib/%.c
	@mkdir -p $(BUILD_DIR)/lib
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -o $@ $(LDFLAGS)

$(BUILD_DIR)/sandbox/%: sandbox/%.c
	@mkdir -p $(BUILD_DIR)/sandbox
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -o $@ $(LDFLAGS)

$(BUILD_DIR)/$(TEST_DIR)/%: $(TEST_DIR)/%.c #lib/**.c
	@mkdir -p $(BUILD_DIR)/$(TEST_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -o $@ $(TEST_LDFLAGS)

.PRECIOUS: $(BUILD_DIR)/$(TEST_DIR)/%

.PHONY: all lib cli build run test test-bin\
		install-lib install-cli install install-headers\
		install-dev uninstall clean