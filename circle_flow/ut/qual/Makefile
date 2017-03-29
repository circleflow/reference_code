TESTS = qual_test

include ../common.mk

bytes_bits.o : $(USER_DIR)/src/bytes_bits.cpp
	$(MAKE_OBJ)

field_parser.o : $(USER_DIR)/src/field/field_parser.cpp
	$(MAKE_OBJ)

field.o : $(USER_DIR)/src/field/field.cpp \
        $(USER_DIR)/export/field/field.h
	$(MAKE_OBJ)

field_impl.o : $(USER_DIR)/src/field/field_impl.cpp \
        $(USER_DIR)/src/field/field_impl.h
	$(MAKE_OBJ)

pkt_lib.o : $(USER_DIR)/src/pkt/pkt_lib.cpp
	$(MAKE_OBJ)

field_seed.o : $(USER_DIR)/src/field/field_seed.cpp \
        $(USER_DIR)/export/field/field_seed.h
	$(MAKE_OBJ)

qual_test.o : ./qual_test.cpp 
	$(MAKE_OBJ)

qual_test : qual_test.o \
            $(STUB_DIR)/tpid_stub.o $(STUB_DIR)/udf_stub.o\
            field.o field_impl.o field_seed.o bytes_bits.o pkt_lib.o field_parser.o\
            $(TEST_DEP)
	$(MAKE_RUN)

