#!/bin/bash

set -e

FIXTURES_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$FIXTURES_DIR"

if ! command -v java >/dev/null 2>&1; then
    echo "ERROR: 'java' is not installed or not in your PATH."
    echo "Java is required to generate the LCG test vectors. Please install a JRE."
    exit 1
fi

if ! command -v javac >/dev/null 2>&1; then
    echo "ERROR: 'javac' (Java Compiler) is not installed."
    echo "A JDK is required to compile the LCG test vector generator."
    exit 1
fi

echo "Compiling LcgTestVectorGenerator.java..."

javac LcgTestVectorGenerator.java

echo "Generating lcg_test_vectors.csv..."

java LcgTestVectorGenerator "lcg_test_vectors.csv"

echo "Cleaning up..."
rm -f LcgTestVectorGenerator.class

echo "Fixture setup complete! results in tests/fixtures/lcg_test_vectors.csv"
