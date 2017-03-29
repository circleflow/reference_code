# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.

TESTS = stub

include ../common.mk

log.o : $(USER_DIR)/src/log.cpp \
          $(USER_DIR)/export/log.h
	$(MAKE_OBJ)

error.o : $(USER_DIR)/src/error.cpp \
          $(USER_DIR)/export/error.h
	$(MAKE_OBJ)

trace.o : $(USER_DIR)/src/trace.cpp \
          $(USER_DIR)/export/trace.h
	$(MAKE_OBJ)

mutex_u.o : $(USER_DIR)/src/mutex_u.cpp \
          $(USER_DIR)/export/mutex_u.h
	$(MAKE_OBJ)

stub : log.o trace.o error.o mutex_u.o
