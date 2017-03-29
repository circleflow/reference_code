#!/bin/sh

export OPMOCK=$OPMOCK_DIR/opmock.sh
export SWIG_FEATURES="$SWIG_FEATURES -I../export"
export SWIG=$SWIG_DIR/swig

$OPMOCK --input-file $USER_DIR/src/mmu/hal_mmu.h \
    --output-path . --use-cpp yes

$OPMOCK_FIX CIRCLE_FLOW::HAL_MMU_SPECS  ./hal_mmu_stub.cpp