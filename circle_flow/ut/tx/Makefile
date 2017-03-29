

TESTS = gen helper_test observer_test

include ../common.mk

mock_pkt_cb_stub.o : ./mock_pkt_cb_stub.cpp
	$(MAKE_OBJ)

helper_test.o : ./helper_test.cpp 
	$(MAKE_OBJ)

observer_test.o : ./observer_test.cpp 
	$(MAKE_OBJ)

helper_test : helper_test.o \
          $(TEST_DEP)
	$(MAKE_RUN)

observer_test : observer_test.o mock_pkt_cb_stub.o\
          $(TEST_DEP)
	$(MAKE_RUN)