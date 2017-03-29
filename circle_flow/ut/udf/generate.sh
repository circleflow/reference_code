#!/bin/sh

export OPMOCK=$OPMOCK_DIR/opmock.sh
export SWIG_FEATURES="$SWIG_FEATURES -I../export"
export SWIG=$SWIG_DIR/swig

$OPMOCK --input-file $USER_DIR/src/udf/hal_udf.h \
    --output-path . --use-cpp yes

$OPMOCK_FIX CIRCLE_FLOW::UDF_HW_SPECS  ./hal_udf_stub.cpp