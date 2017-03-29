
TESTS = gen port_test

include ../common.mk

port.o : $(USER_DIR)/src/port/port.cpp
	$(MAKE_OBJ)

counter.o : $(USER_DIR)/src/port/counter.cpp
	$(MAKE_OBJ)

notify.o : $(USER_DIR)/src/port/notify.cpp
	$(MAKE_OBJ)

hal_port_stub.o : ./hal_port_stub.cpp
	$(MAKE_OBJ)

mock_notify_stub.o : ./mock_notify_stub.cpp
	$(MAKE_OBJ)

port_test.o : ./port_test.cpp 
	$(MAKE_OBJ)

port_test : port.o counter.o notify.o hal_port_stub.o port_test.o mock_notify_stub.o \
            $(STUB_DIR)/timer_mock.o $(STUB_DIR)/timer_mock_stub.o \
            $(TEST_DEP)
	$(MAKE_RUN)
