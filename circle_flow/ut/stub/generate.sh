#!/bin/sh

export OPMOCK=$OPMOCK_DIR/opmock.sh
export SWIG_FEATURES="$SWIG_FEATURES -I../export"
export SWIG=$SWIG_DIR/swig

$OPMOCK --input-file $USER_DIR/src/include/cpu.h \
    --output-path . --use-cpp yes

$OPMOCK --input-file $USER_DIR/src/include/hal.h \
    --output-path . --use-cpp yes
$OPMOCK_FIX "vector<UINT8>"  ./hal_stub.cpp
$OPMOCK_FIX "CIRCLE_FLOW::HAL_UNIT_SPECS"  ./hal_stub.cpp
$OPMOCK_FIX "vector<int>"  ./hal_stub.cpp

$OPMOCK --input-file $USER_DIR/src/include/mmu.h \
    --output-path . --use-cpp yes

$OPMOCK --input-file $USER_DIR/src/include/port_impl.h \
    --output-path . --use-cpp yes

$OPMOCK --input-file $USER_DIR/src/include/queue.h \
    --output-path . --use-cpp yes

$OPMOCK --input-file $USER_DIR/src/include/rx.h \
    --output-path . --use-cpp yes
    
$OPMOCK --input-file $USER_DIR/src/include/tpid.h \
    --output-path . --use-cpp yes

$OPMOCK --input-file $USER_DIR/src/include/tx.h \
    --output-path . --use-cpp yes

$OPMOCK --input-file $USER_DIR/src/include/udf.h \
    --output-path . --use-cpp yes

$OPMOCK --input-file ./unique_mock.h \
    --output-path . --use-cpp yes

$OPMOCK --input-file $USER_DIR/export/port.h \
    --output-path . --use-cpp yes

$OPMOCK --input-file ./timer_mock.h \
    --output-path . --use-cpp yes
    