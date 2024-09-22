#!/bin/bash

# Step 1: Process CSV files
echo "Step 1: Processing CSV files using processCsv.py..."
python3 ./src/processCsv.py
if [ $? -ne 0 ]; then
    echo "Error: Failed to process CSV files."
    exit 1
fi
echo "CSV files processed successfully."

# Step 2: Compile the C++ program
echo "Step 2: Compiling the C++ program with g++..."
g++ -Ofast -o main.exe src/main.cpp
if [ $? -ne 0 ]; then
    echo "Error: Compilation failed."
    exit 1
fi
echo "Compilation successful."

# Step 3: Run the compiled program
echo "Step 3: Running the compiled program..."
./main.exe
if [ $? -ne 0 ]; then
    echo "Error: Execution of main.exe failed."
    exit 1
fi
echo "Program executed successfully."

