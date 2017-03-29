#!/bin/sh

export OPMOCK=$OPMOCK_DIR/opmock.sh
export SWIG_FEATURES="$SWIG_FEATURES -I../export"
export SWIG=$SWIG_DIR/swig

$OPMOCK --input-file ./mock_pkt_cb.h \
    --output-path . --use-cpp yes