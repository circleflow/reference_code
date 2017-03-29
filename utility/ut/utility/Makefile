# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.

TESTS = unique_test random_test timer_test 

include ../common.mk

thread_u.o : $(USER_DIR)/src/thread_u.cpp \
          $(USER_DIR)/export/thread_u.h
	$(MAKE_OBJ)

time_u.o : $(USER_DIR)/src/time_u.cpp \
          $(USER_DIR)/export/time_u.h
	$(MAKE_OBJ)

timer.o : $(USER_DIR)/src/timer.cpp \
          $(USER_DIR)/export/timer.h
	$(MAKE_OBJ)


timer_test.o : ./timer_test.cpp
	$(MAKE_OBJ)

timer_test : timer.o timer_test.o \
            thread_u.o time_u.o \
            $(TEST_DEP)
	$(MAKE_RUN)

unique_test.o : ./unique_test.cpp
	$(MAKE_OBJ)

unique_test : unique_test.o \
            $(TEST_DEP)
	$(MAKE_RUN)

random_u.o : $(USER_DIR)/src/random_u.cpp
	$(MAKE_OBJ)

random_test.o : ./random_test.cpp
	$(MAKE_OBJ)

random_test : random_u.o random_test.o \
            $(TEST_DEP)
	$(MAKE_RUN)
