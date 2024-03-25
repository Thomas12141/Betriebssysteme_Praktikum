#!/bin/sh

#========== Check for dependencies [start] ==========#
# Check if doxygen is installed
if ! command -v doxygen >/dev/null 2>&1; then
    echo "doxygen could not be found (try to install with: sudo apt-get install doxygen)"
    exit 1
fi

# Check if make is installed
if ! command -v make >/dev/null 2>&1; then
    echo "make could not be found (try to install with: sudo apt-get install make)"
    exit 1
fi

# Check if pdflatex is installed
if ! command -v pdflatex >/dev/null 2>&1; then
    echo "pdflatex could not be found (try to install with: sudo apt-get install texlive-latex-base)"
    exit 1
fi

# Check if dot (part of Graphviz) is installed
if ! command -v dot >/dev/null 2>&1; then
    echo "Graphviz could not be found (try to install with: sudo apt-get install graphviz)"
    exit 1
fi
#========== Check for dependencies [end]   ==========#


#========== CWD GUARD [start] ==========#
PROJECT_DIR=$(realpath "$(dirname "$0")/..")
CALL_DIR=$(pwd)

# Change to the project directory
cd "$PROJECT_DIR" || exit 1
#========== CWD GUARD [end]   ==========#


#========== Variables [start] ==========#
DOXYFILE="./docs/Doxyfile"
PDF_NAME="OSMP_Documentation"
#========== Variables [end]   ==========#


# Create the doxygen documentation
export PROJECT_NUMBER="Erstellt am $(date +'%d.%m.%Y')."
doxygen ${DOXYFILE}

cd ./docs/latex || { rm -r ./latex; rm -r ./html; exit 1; }

# Create the pdf documentation
echo "--- Creating the pdf documentation ---"
make > /dev/null || { cd ..; rm -r ./latex; rm -r ./html; exit 1; }

# Move the pdf documentation to the docs directory
mv ./refman.pdf ../$PDF_NAME.pdf

# Delete unnecessary files
cd ..
rm -r ./latex
rm -r ./html


#========== CWD GUARD [start] ==========#
# Change back to the call directory
cd "$CALL_DIR" || exit 1
#========== CWD GUARD [end]   ==========#