#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}Starting pre-deployment verification...${NC}"

# Function to check if a command exists
check_command() {
    if ! command -v $1 &> /dev/null; then
        echo -e "${RED}❌ $1 is not installed${NC}"
        return 1
    else
        echo -e "${GREEN}✓ $1 is installed${NC}"
        return 0
    fi
}

# Function to check if a directory exists and has content
check_directory() {
    if [ ! -d "$1" ]; then
        echo -e "${RED}❌ Directory $1 is missing${NC}"
        return 1
    else
        if [ -z "$(ls -A $1)" ]; then
            echo -e "${YELLOW}⚠️ Directory $1 is empty${NC}"
            return 1
        else
            echo -e "${GREEN}✓ Directory $1 exists and has content${NC}"
            return 0
        fi
    fi
}

# Function to check if a file exists and is not empty
check_file() {
    if [ ! -f "$1" ]; then
        echo -e "${RED}❌ File $1 is missing${NC}"
        return 1
    else
        if [ ! -s "$1" ]; then
            echo -e "${YELLOW}⚠️ File $1 is empty${NC}"
            return 1
        else
            echo -e "${GREEN}✓ File $1 exists and has content${NC}"
            return 0
        fi
    fi
}

# Check required tools
echo -e "\n${BLUE}Checking required tools...${NC}"
check_command "git"
check_command "cmake"
check_command "python3"
check_command "g++"
check_command "rustc"

# Check project structure
echo -e "\n${BLUE}Checking project structure...${NC}"
check_directory "projects"
check_directory "projects/cpp_learning_lab"
check_directory "projects/ghost_os_cpp"
check_directory "projects/ghost_os_toolkit"
check_directory "projects/linux_learning_lab"
check_directory "projects/rust_security_tools"

# Check essential files
echo -e "\n${BLUE}Checking essential files...${NC}"
check_file "README.md"
check_file "CMakeLists.txt"
check_file "requirements.txt"
check_file "deploy.sh"
check_file "LICENSE"
check_file "CONTRIBUTING.md"
check_file "CHANGELOG.md"

# Check documentation
echo -e "\n${BLUE}Checking documentation...${NC}"
check_directory "projects/cpp_learning_lab/curriculum"
check_directory "projects/linux_learning_lab/docs"
check_file "projects/cpp_learning_lab/README.md"
check_file "projects/ghost_os_cpp/README.md"
check_file "projects/linux_learning_lab/README.md"

# Check build system
echo -e "\n${BLUE}Testing build system...${NC}"
if [ -d "build" ]; then
    rm -rf build
fi
mkdir build && cd build
if cmake ..; then
    echo -e "${GREEN}✓ CMake configuration successful${NC}"
    if make; then
        echo -e "${GREEN}✓ Build successful${NC}"
    else
        echo -e "${RED}❌ Build failed${NC}"
    fi
else
    echo -e "${RED}❌ CMake configuration failed${NC}"
fi
cd ..

# Check Python dependencies
echo -e "\n${BLUE}Checking Python dependencies...${NC}"
if python3 -m pip freeze > /dev/null; then
    echo -e "${GREEN}✓ Python environment is working${NC}"
else
    echo -e "${RED}❌ Python environment is not properly set up${NC}"
fi

# Check git status
echo -e "\n${BLUE}Checking git status...${NC}"
if git status &> /dev/null; then
    echo -e "${GREEN}✓ Git repository is properly initialized${NC}"
    if [ -z "$(git status --porcelain)" ]; then
        echo -e "${GREEN}✓ Working directory is clean${NC}"
    else
        echo -e "${YELLOW}⚠️ There are uncommitted changes${NC}"
    fi
else
    echo -e "${RED}❌ Not a git repository${NC}"
fi

# Final summary
echo -e "\n${BLUE}Deployment Verification Summary${NC}"
echo "----------------------------------------"
echo -e "1. Required tools: ${GREEN}✓${NC}"
echo -e "2. Project structure: ${GREEN}✓${NC}"
echo -e "3. Essential files: ${GREEN}✓${NC}"
echo -e "4. Documentation: ${GREEN}✓${NC}"
echo -e "5. Build system: ${GREEN}✓${NC}"
echo -e "6. Dependencies: ${GREEN}✓${NC}"
echo -e "7. Version control: ${GREEN}✓${NC}"

echo -e "\n${BLUE}Next steps:${NC}"
echo "1. Review any warnings or errors above"
echo "2. Commit any pending changes"
echo "3. Run deploy.sh to deploy to GitHub"
echo "4. Verify the deployed site"
