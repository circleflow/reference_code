
TESTS = gen mmu_test

include ../common.mk

mmu.o : $(USER_DIR)/src/mmu/mmu.cpp \
        $(USER_DIR)/src/include/mmu.h
	$(MAKE_OBJ)

hal_mmu_stub.o : ./hal_mmu_stub.cpp
	$(MAKE_OBJ)

mmu_test.o : ./mmu_test.cpp 
	$(MAKE_OBJ)

mmu_test : mmu.o hal_mmu_stub.o mmu_test.o \
            $(STUB_DIR)/port_impl_stub.o\
            $(TEST_DEP)
	$(MAKE_RUN)

