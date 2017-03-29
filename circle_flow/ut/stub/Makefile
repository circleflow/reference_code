
TESTS = stub

include ../common.mk

cpu_stub.o : ./cpu_stub.cpp
	$(MAKE_OBJ)

hal_stub.o : ./hal_stub.cpp
	$(MAKE_OBJ)

mmu_stub.o : ./mmu_stub.cpp
	$(MAKE_OBJ)

port_impl_stub.o : ./port_impl_stub.cpp
	$(MAKE_OBJ)

queue_stub.o : ./queue_stub.cpp
	$(MAKE_OBJ)

rx_stub.o : ./rx_stub.cpp
	$(MAKE_OBJ)

tpid_stub.o : ./tpid_stub.cpp
	$(MAKE_OBJ)

tx_stub.o : ./tx_stub.cpp
	$(MAKE_OBJ)

udf_stub.o : ./udf_stub.cpp
	$(MAKE_OBJ)

port_stub.o : ./port_stub.cpp
	$(MAKE_OBJ)

field_stub.o : ./field_stub.cpp
	$(MAKE_OBJ)

field_block_stub.o : ./field_block_stub.cpp
	$(MAKE_OBJ)


defs.o : $(USER_DIR)/src/defs.cpp
	$(MAKE_OBJ)

trace.o : $(SRC_DIR)/utility/src/trace.cpp \
          $(SRC_DIR)/utility/export/trace.h
	$(MAKE_OBJ)

error.o : $(SRC_DIR)/utility/src/error.cpp \
          $(SRC_DIR)/utility/export/error.h
	$(MAKE_OBJ)

mutex_u.o : $(SRC_DIR)/utility/src/mutex_u.cpp \
               $(SRC_DIR)/utility/export/mutex_u.h
	$(MAKE_OBJ)

log.o :  $(SRC_DIR)/utility/src/log.cpp \
         $(SRC_DIR)/utility/export/log.h
	$(MAKE_OBJ)
	
timer_mock.o : ./timer_mock.cpp
	$(MAKE_OBJ)

timer_mock_stub.o : ./timer_mock_stub.cpp
	$(MAKE_OBJ)


stub :  gen \
       cpu_stub.o hal_stub.o mmu_stub.o port_impl_stub.o queue_stub.o rx_stub.o tpid_stub.o tx_stub.o udf_stub.o \
       mutex_u.o log.o port_stub.o trace.o error.o  defs.o\
       timer_mock.o timer_mock_stub.o
