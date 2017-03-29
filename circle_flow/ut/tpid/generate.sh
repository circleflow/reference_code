#!/bin/sh

export OPMOCK=$OPMOCK_DIR/opmock.sh
export SWIG_FEATURES="$SWIG_FEATURES -I../export"
export SWIG=$SWIG_DIR/swig

$OPMOCK --input-file $USER_DIR/src/tpid/hal_tpid.h \
    --output-path . --use-cpp yes
