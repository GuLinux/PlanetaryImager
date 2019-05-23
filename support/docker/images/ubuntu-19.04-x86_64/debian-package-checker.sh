#!/bin/bash
PACKAGE="$1"

if ! [[ -r "$PACKAGE" ]]; then
    echo "Unable to find package to test: $PACKAGE" >&2
    exit 1
fi

echo "**** Testing installation for $PACKAGE"
apt install -y "./$PACKAGE"

