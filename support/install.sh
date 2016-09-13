#!/bin/bash
# Script for installing development snapshots (TGZ/TBZ2) to system
basedir="$( dirname "$( readlink -f "$0")" )"

for file in "$basedir/"*; do
  [[ -d "$file" ]] && cp -av "$file" / # TODO: stronger checks
done
