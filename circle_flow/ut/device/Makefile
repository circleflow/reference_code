
TESTS = device_test

include ../common.mk

device_test.o : ./device_test.cpp \
                $(USER_DIR)/export/device.h
	$(MAKE_OBJ)

device_test : device_test.o \
              $(TEST_DEP)
	$(MAKE_RUN)

