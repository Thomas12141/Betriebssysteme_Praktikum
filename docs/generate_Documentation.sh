#!/bin/sh

# Check if doxygen is installed
if ! command -v doxygen >/dev/null 2>&1; then
    echo "doxygen could not be found (install with: `sudo apt-get install doxygen`)"
    exit 1
fi

# Check if make is installed
if ! command -v make >/dev/null 2>&1; then
    echo "make could not be found (install with: `sudo apt-get install make`)"
    exit 1
fi

# Check if pdflatex is installed
if ! command -v pdflatex >/dev/null 2>&1; then
    echo "pdflatex could not be found (install with: `sudo apt-get install texlive-latex-base`)"
    exit 1
fi

# Check if dot (part of Graphviz) is installed
if ! command -v dot >/dev/null 2>&1; then
    echo "Graphviz could not be found (install with: `sudo apt-get install graphviz`)"
    exit 1
fi

PROJECT_DIR=$(realpath "$(dirname "$0")/..")
CALL_DIR=$(pwd)

# Change to the project directory
cd "$PROJECT_DIR" || exit 1

# Create the doxygen documentation
doxygen ./docs/Doxyfile

cd ./docs/latex || exit 1

# Create the pdf documentation
make

# Move the pdf documentation to the docs directory
mv ./refman.pdf ../OSMP_Documentation.pdf

# Delete unnecessary files
cd ..
rm -r ./latex
# Maybe we want to keep the html documentation
rm -r ./html

# Change back to the call directory
cd "$CALL_DIR" || exit 1