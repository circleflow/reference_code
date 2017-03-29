
GTEST_DIR  = ../../../ut_framework/gtest
OPMOCK_DIR = ../../../ut_framework/opmock
SWIG_DIR   = ../../../ut_framework/swig

OPMOCK_FIX = $(OPMOCK_DIR)/fix_ref_return.sh

# Where to find user code.
USER_DIR = ../..
INC_DIR  = ../../..
# export for generate.sh
export OPMOCK_DIR SWIG_DIR USER_DIR OPMOCK_FIX

STUB_DIR = ../stub
TEST_DIR = ../test

INC_FLAG = -I$(USER_DIR)/export -I$(USER_DIR)/ut -I$(USER_DIR)/src
INC_FLAG += -I$(INC_DIR)
DEF_FLAG = -DENV_UT -DENV_C11

# flags to test framework
CPPFLAGS += -isystem $(GTEST_DIR)/include -I$(GTEST_DIR) -I$(OPMOCK_DIR)/support 
CPPFLAGS += -O0  -ggdb --coverage -g -Wall -Wextra -std=gnu++11 -DGTEST_LANG_CXX11

# Flags to c++ test file
CXXFLAGS +=  $(INC_FLAG) $(DEF_FLAG)

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
			$(STUB_DIR)/trace.o $(STUB_DIR)/error.o $(STUB_DIR)/log.o $(STUB_DIR)/mutex_u.o

