#!/bin/sh

export OPMOCK=$OPMOCK_DIR/opmock.sh
export SWIG_FEATURES="$SWIG_FEATURES -I../export"
export SWIG=$SWIG_DIR/swig


$OPMOCK --input-file $USER_DIR/src/cpu/cpu_hal.h \
    --output-path . --use-cpp yes

$OPMOCK --input-file ./mock_pkt_rx.h \
    --output-path . --use-cpp yes