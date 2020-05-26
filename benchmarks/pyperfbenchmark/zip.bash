#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <benchmark_name>"
    exit 1
fi

# First argument to script is file name to be created
# eg. bm_unpack_sequence.zip
ZIP_NAME=$1".zip"

SITE=virtualenv/lib/python3.6/site-packages
VENV=virtualenv/bin/activate_this.py
MAIN=__main__.py
PYPERF=$SITE/pyperf
SIX=$SITE/six.py

# Create chain of arguments
ARG=$VENV
ARG+=" "$MAIN
ARG+=" "$PYPERF
ARG+=" "$SIX

# This command creates the zip archive needed!
zip -r $ZIP_NAME $ARG

