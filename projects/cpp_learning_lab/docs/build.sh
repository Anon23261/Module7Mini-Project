#!/bin/bash

# Exit on error
set -e

# Install doxygen if not present
if ! command -v doxygen &> /dev/null; then
    sudo apt-get update
    sudo apt-get install -y doxygen graphviz
fi

# Create docs directory if it doesn't exist
mkdir -p docs/html

# Generate Doxyfile if it doesn't exist
if [ ! -f Doxyfile ]; then
    doxygen -g
    # Configure Doxyfile
    sed -i 's/PROJECT_NAME.*=.*/PROJECT_NAME = "C++ Learning Lab"/' Doxyfile
    sed -i 's/OUTPUT_DIRECTORY.*=.*/OUTPUT_DIRECTORY = docs/' Doxyfile
    sed -i 's/GENERATE_HTML.*=.*/GENERATE_HTML = YES/' Doxyfile
    sed -i 's/EXTRACT_ALL.*=.*/EXTRACT_ALL = YES/' Doxyfile
    sed -i 's/EXTRACT_PRIVATE.*=.*/EXTRACT_PRIVATE = YES/' Doxyfile
    sed -i 's/EXTRACT_STATIC.*=.*/EXTRACT_STATIC = YES/' Doxyfile
    sed -i 's/RECURSIVE.*=.*/RECURSIVE = YES/' Doxyfile
fi

# Run doxygen
doxygen Doxyfile
