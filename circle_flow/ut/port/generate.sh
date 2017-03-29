#!/bin/sh

export OPMOCK=$OPMOCK_DIR/opmock.sh
export SWIG_FEATURES="$SWIG_FEATURES -I../export"
export SWIG=$SWIG_DIR/swig

$OPMOCK --input-file $USER_DIR/src/port/hal_port.h \
    --output-path . --use-cpp yes

$OPMOCK --input-file ./mock_notify.h \
    --output-path . --use-cpp yes