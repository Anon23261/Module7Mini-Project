#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}Starting deployment process...${NC}"

# Check if git is installed
if ! command -v git &> /dev/null; then
    echo -e "${RED}Git is not installed. Please install git first.${NC}"
    exit 1
fi

# Check if repository exists
if [ ! -d .git ]; then
    echo -e "${BLUE}Initializing git repository...${NC}"
    git init
    git add .
    git commit -m "Initial commit"
fi

# Check if remote exists
if ! git remote | grep -q "^origin$"; then
    echo -e "${RED}No remote repository found.${NC}"
    echo "Please enter your GitHub repository URL:"
    read -r REPO_URL
    git remote add origin "$REPO_URL"
fi

# Build process
echo -e "${BLUE}Building project...${NC}"

# Create dist directory
mkdir -p dist

# Copy necessary files
cp index.html dist/
cp styles.css dist/
cp script.js dist/
cp -r assets dist/
cp -r projects dist/

# Optimize images
if command -v optipng &> /dev/null; then
    echo -e "${BLUE}Optimizing PNG images...${NC}"
    find dist/assets -name "*.png" -exec optipng -o5 {} \;
fi

if command -v jpegoptim &> /dev/null; then
    echo -e "${BLUE}Optimizing JPEG images...${NC}"
    find dist/assets -name "*.jpg" -exec jpegoptim --max=85 {} \;
fi

# Minify CSS
if command -v cleancss &> /dev/null; then
    echo -e "${BLUE}Minifying CSS...${NC}"
    cleancss -o dist/styles.min.css dist/styles.css
    mv dist/styles.min.css dist/styles.css
fi

# Minify JavaScript
if command -v uglifyjs &> /dev/null; then
    echo -e "${BLUE}Minifying JavaScript...${NC}"
    uglifyjs dist/script.js -o dist/script.min.js
    mv dist/script.min.js dist/script.js
fi

# Deploy to GitHub Pages
echo -e "${BLUE}Deploying to GitHub Pages...${NC}"

# Create or update gh-pages branch
if git show-ref --quiet refs/heads/gh-pages; then
    git branch -D gh-pages
fi

git checkout -b gh-pages

# Move dist contents to root
mv dist/* .
rm -rf dist

# Add and commit changes
git add .
git commit -m "Deploy to GitHub Pages"

# Push to GitHub
echo -e "${BLUE}Pushing to GitHub...${NC}"
git push origin gh-pages --force

# Switch back to main branch
git checkout main

echo -e "${GREEN}Deployment complete!${NC}"
echo -e "${GREEN}Your website should be available at: https://yourusername.github.io/repository-name${NC}"
echo -e "${BLUE}Note: It may take a few minutes for the changes to propagate.${NC}"
