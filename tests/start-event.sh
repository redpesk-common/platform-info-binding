#!/usr/bin/env bash
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

sudo LD_LIBRARY_PATH=$SCRIPT_DIR/../build python $SCRIPT_DIR/test-event.py -vvv



