
TESTS =  bytes_bits_test 

include ../common.mk


bytes_bits.o : $(USER_DIR)/src/bytes_bits.cpp \
               $(USER_DIR)/src/include/bytes_bits.h
	$(MAKE_OBJ)

bytes_bits_test.o : ./bytes_bits_test.cpp
	$(MAKE_OBJ)


bytes_bits_test : bytes_bits_test.o bytes_bits.o \
            $(TEST_DEP)
	$(MAKE_RUN)
