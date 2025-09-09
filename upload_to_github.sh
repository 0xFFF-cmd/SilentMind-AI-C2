#!/bin/bash

# SilentMind GitHub Upload Script
# This script will upload the entire project to GitHub

echo "🚀 SilentMind GitHub Upload Script"
echo "=================================="
echo ""

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

print_step() {
    echo -e "${YELLOW}[STEP]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "app.py" ] || [ ! -f "client/main.c" ]; then
    print_error "Please run this script from the SilentMind root directory"
    exit 1
fi

# Check if git is installed
if ! command -v git &> /dev/null; then
    print_error "Git is not installed. Please install git first."
    exit 1
fi

print_step "1" "Initializing git repository..."
git init

print_step "2" "Adding all files..."
git add .

print_step "3" "Creating initial commit..."
git commit -m "Initial commit: SilentMind AI-C2 tool

- AI-powered Command & Control system
- Intelligent file exfiltration with LLM decision making
- Firefox cookie extraction with SQLite parsing
- XOR encryption for secure data transmission
- Cross-platform compilation support
- Comprehensive documentation and demo scripts"

print_step "4" "Setting up remote repository..."
echo "Please enter your GitHub repository URL:"
echo "Example: https://github.com/0xFFF-cmd/SilentMind-AI-C2.git"
read -p "Repository URL: " REPO_URL

if [ -z "$REPO_URL" ]; then
    print_error "Repository URL is required"
    exit 1
fi

git remote add origin "$REPO_URL"

print_step "5" "Pushing to GitHub..."
git branch -M main
git push -u origin main

if [ $? -eq 0 ]; then
    print_success "Repository uploaded successfully!"
    echo ""
    echo "Your repository is now available at:"
    echo "$REPO_URL"
    echo ""
    echo "Next steps:"
    echo "1. Visit your repository on GitHub"
    echo "2. Check that all files are uploaded"
    echo "3. The README.md will display automatically"
    echo "4. Share your repository with others!"
else
    print_error "Failed to push to GitHub. Please check your repository URL and try again."
fi
