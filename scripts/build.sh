#!/bin/bash

# --- Color Definitions ---
RED='\033[31m'
GREEN='\033[32m'
YELLOW='\033[33m'
BLUE='\033[34m'
CYAN='\033[36m'
BOLD='\033[1m'
RESET='\033[0m'

SEARCH_DIRS="backend/include interp/include middleend/include modules"

# Execute Build
cmake --build build -j1

# Success Check 
if [ $? -eq 0 ]; then
    cp build/ry bin
    cp build/*.so lib
    echo -e "\n${GREEN}${BOLD} Build complete.${RESET}"
else
    echo -e "\n${RED}${BOLD} Build failed.${RESET} Check the errors above."
    exit 1
fi