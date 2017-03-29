
TESTS = gen vfp_rx_test

include ../common.mk

vfp_rx.o : $(USER_DIR)/src/fp/vfp_rx.cpp
	$(MAKE_OBJ)

vfp_rx_entry_mock_stub.o : ./vfp_rx_entry_mock_stub.cpp
	$(MAKE_OBJ)

vfp_rx_test.o : ./vfp_rx_test.cpp 
	$(MAKE_OBJ)

vfp_rx_test : vfp_rx.o vfp_rx_test.o vfp_rx_entry_mock_stub.o \
            $(TEST_DEP)
	$(MAKE_RUN)

