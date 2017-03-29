
GTEST_DIR  = ../../../ut_framework/gtest
OPMOCK_DIR = ../../../ut_framework/opmock
SWIG_DIR   = ../../../ut_framework/swig

OPMOCK_FIX = $(OPMOCK_DIR)/fix_ref_return.sh

# Where to find user code.
USER_DIR = ../..
SRC_DIR  = ../../..
INC_DIR  = ../../..
# export for generate.sh
export OPMOCK_DIR SWIG_DIR USER_DIR SRC_DIR INC_DIR OPMOCK_FIX

STUB_DIR = ../stub
TEST_DIR = ../test

INC_FLAG = -I$(USER_DIR)/export -I$(USER_DIR)/export/field -I$(USER_DIR)/export/pkt
INC_FLAG += -I$(USER_DIR)/ut -I$(USER_DIR)/src -I$(USER_DIR)/src/cpu -I$(USER_DIR)/src/field -I$(USER_DIR)/src/fp -I$(USER_DIR)/src/include -I$(USER_DIR)/src/pkt -I$(USER_DIR)/src/mmu -I$(USER_DIR)/src/port -I$(USER_DIR)/src/queue -I$(USER_DIR)/src/tpid -I$(USER_DIR)/src/udf
INC_FLAG += -I$(INC_DIR) -I$(INC_DIR)/utility/export
DEF_FLAG = -DENV_UT -DENV_C11 -DCF_BCM_56334

# Flags pass to test framework
CPPFLAGS += -isystem $(GTEST_DIR)/include -I$(OPMOCK_DIR)/support -I$(GTEST_DIR)
CPPFLAGS += -O0  -ggdb --coverage -g -Wall -Wextra -std=gnu++11 -DGTEST_LANG_CXX11

# Flags passed to the C++ compiler.
CXXFLAGS += $(INC_FLAG) $(DEF_FLAG)



all : $(TESTS)

gen:
	./generate.sh

clean:
	rm -f *_stub.*
	rm -f *.xml
	rm -f *.o
	rm -f *.gcno
	rm -f *.gcda
	rm -f *.ut
	rm -f *.*.stackdump
	rm -f core
	rm -f *.a

MAKE_OBJ = $(CXX) $(CPPFLAGS) $(CXXFLAGS) -I../stub -I../test -c $<
MAKE_RUN = $(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@.ut; ./$@.ut
TEST_DEP = $(TEST_DIR)/gtest_main.a $(OPMOCK_DIR)/support/opmock.o \
		   $(STUB_DIR)/defs.o  $(STUB_DIR)/mutex_u.o $(STUB_DIR)/error.o $(STUB_DIR)/trace.o $(STUB_DIR)/log.o  
