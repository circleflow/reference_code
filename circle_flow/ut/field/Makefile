
TESTS = gen field_test parser_test field_seed_test

include ../common.mk

field_parser.o : $(USER_DIR)/src/field/field_parser.cpp \
        $(USER_DIR)/export/field/field_parser.h
	$(MAKE_OBJ)

field.o : $(USER_DIR)/src/field/field.cpp \
        $(USER_DIR)/export/field/field.h
	$(MAKE_OBJ)

field_impl.o : $(USER_DIR)/src/field/field_impl.cpp \
        $(USER_DIR)/src/field/field_impl.h
	$(MAKE_OBJ)

bytes_bits.o : $(USER_DIR)/src/bytes_bits.cpp
	$(MAKE_OBJ)

mock_cac_stub.o : ./mock_cac_stub.cpp
	$(MAKE_OBJ)

pkt_lib.o : $(USER_DIR)/src/pkt/pkt_lib.cpp
	$(MAKE_OBJ)

field_seed.o : $(USER_DIR)/src/field/field_seed.cpp \
        $(USER_DIR)/export/field/field_seed.h
	$(MAKE_OBJ)

parser_test.o : ./parser_test.cpp 
	$(MAKE_OBJ)

field_seed_test.o : ./field_seed_test.cpp 
	$(MAKE_OBJ)

field_test.o : ./field_test.cpp 
	$(MAKE_OBJ)


field_seed_test : field_seed.o bytes_bits.o field_seed_test.o \
            $(TEST_DEP)
	$(MAKE_RUN)

parser_test : field_parser.o parser_test.o \
            $(TEST_DEP)
	$(MAKE_RUN)

field_test : field.o field_impl.o field_seed.o field_test.o bytes_bits.o pkt_lib.o mock_cac_stub.o field_parser.o\
            $(TEST_DEP)
	$(MAKE_RUN)

