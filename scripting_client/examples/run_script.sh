#!/bin/bash
export PYTHONPATH="$( cd "$( dirname "$0" )/.." && pwd):$PYTHONPATH"
PYTHON_BIN="${PYTHON_BIN:-/usr/bin/python3}"


if [ -z "$1" ] || ! [ -r "$1" ]; then
    echo "Usage: $0 python-file [options]"
    echo "Runs a PlanetaryImager script setting the correct PYTHONPATH environment variable"
    exit 1
fi


$PYTHON_BIN "$@"

