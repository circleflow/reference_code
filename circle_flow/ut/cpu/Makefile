
TESTS = gen cpu_test

include ../common.mk

cpu.o : $(USER_DIR)/src/cpu/cpu.cpp \
        $(USER_DIR)/src/include/cpu.h
	$(MAKE_OBJ)

cpu_hal_stub.o : ./cpu_hal_stub.cpp
	$(MAKE_OBJ)

mock_pkt_rx_stub.o : ./mock_pkt_rx_stub.cpp
	$(MAKE_OBJ)

cpu_test.o : ./cpu_test.cpp 
	$(MAKE_OBJ)

cpu_test : cpu.o cpu_hal_stub.o cpu_test.o mock_pkt_rx_stub.o \
            $(STUB_DIR)/queue_stub.o $(STUB_DIR)/hal_stub.o $(STUB_DIR)/port_impl_stub.o \
            $(TEST_DEP)
	$(MAKE_RUN)

