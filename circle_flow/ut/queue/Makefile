
TESTS = gen queue_test

include ../common.mk

queue.o : $(USER_DIR)/src/queue/queue.cpp \
        $(USER_DIR)/src/include/queue.h
	$(MAKE_OBJ)

hal_queue_stub.o : ./hal_queue_stub.cpp
	$(MAKE_OBJ)

queue_test.o : ./queue_test.cpp 
	$(MAKE_OBJ)

queue_test : queue.o hal_queue_stub.o queue_test.o \
            $(TEST_DEP)
	$(MAKE_RUN)

