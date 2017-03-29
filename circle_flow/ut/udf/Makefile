

TESTS = gen udf_test

include ../common.mk

udf.o : $(USER_DIR)/src/udf/udf.cpp
	$(MAKE_OBJ)

hal_udf_stub.o : ./hal_udf_stub.cpp
	$(MAKE_OBJ)

udf_test.o : ./udf_test.cpp 
	$(MAKE_OBJ)


udf_test : udf.o hal_udf_stub.o udf_test.o \
           $(TEST_DEP)
	$(MAKE_RUN)

