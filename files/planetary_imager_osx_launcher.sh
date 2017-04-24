#!/bin/bash
current_dir="$( cd "$( dirname "$0" )" && pwd )"
bundle_dir="$( cd "$current_dir/.." && pwd )"
"$bundle_dir"/bin/planetary_imager --drivers "$bundle_dir/lib/PlanetaryImager/drivers" "$@"

