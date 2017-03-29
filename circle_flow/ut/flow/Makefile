
TESTS = gen flow_track_test latency_cac_test

include ../common.mk

flow_track_test.o : ./flow_track_test.cpp 
	$(MAKE_OBJ)

flow_track.o : $(USER_DIR)/src/flow/flow_track.cpp
	$(MAKE_OBJ)

bytes_bits.o : $(USER_DIR)/src/bytes_bits.cpp
	$(MAKE_OBJ)

field_parser.o : $(USER_DIR)/src/field/field_parser.cpp \
        $(USER_DIR)/export/field/field_parser.h
	$(MAKE_OBJ)

field.o : $(USER_DIR)/src/field/field.cpp \
        $(USER_DIR)/export/field/field.h
	$(MAKE_OBJ)

field_impl.o : $(USER_DIR)/src/field/field_impl.cpp \
        $(USER_DIR)/src/field/field_impl.h
	$(MAKE_OBJ)

field_seed.o : $(USER_DIR)/src/field/field_seed.cpp \
        $(USER_DIR)/export/field/field_seed.h
	$(MAKE_OBJ)

pkt_lib.o : $(USER_DIR)/src/pkt/pkt_lib.cpp
	$(MAKE_OBJ)

flow_track_test : flow_track_test.o flow_track.o  bytes_bits.o \
                  field.o field_impl.o field_parser.o field_seed.o pkt_lib.o \
                  $(TEST_DEP)
	$(MAKE_RUN)


latency_cac_test.o : ./latency_cac_test.cpp
	$(MAKE_OBJ)

pkt_gen_mock_stub.o : ./pkt_gen_mock_stub.cpp
	$(MAKE_OBJ)

latency_cac_test : latency_cac_test.o pkt_gen_mock_stub.o \
                  $(STUB_DIR)/timer_mock.o $(STUB_DIR)/timer_mock_stub.o \
                  $(TEST_DEP)
	$(MAKE_RUN)