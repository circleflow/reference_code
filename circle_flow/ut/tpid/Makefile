
TESTS = gen tpid_test

include ../common.mk

tpid.o : $(USER_DIR)/src/tpid/tpid.cpp $(USER_DIR)/src/include/tpid.h
	$(MAKE_OBJ)

tpid_test.o : ./tpid_test.cpp \
                $(USER_DIR)/src/include/tpid.h
	$(MAKE_OBJ)

hal_tpid_stub.o : ./hal_tpid_stub.cpp \
             $(USER_DIR)/src/tpid/hal_tpid.h
	$(MAKE_OBJ)

tpid_test : tpid.o tpid_test.o hal_tpid_stub.o \
            $(STUB_DIR)/port_impl_stub.o $(STUB_DIR)/hal_stub.o \
            $(TEST_DEP)
	$(MAKE_RUN)

