#!/bin/bash

echo "Starting Closed Loop Motor Configuration Tool..."
echo

if [ ! -d "venv" ]; then
    echo "Creating virtual environment..."
    python3 -m venv venv
    echo
fi

echo "Activating virtual environment..."
source venv/bin/activate

echo "Installing dependencies..."
pip install -q -r requirements.txt

echo
echo "Starting application..."
python main.py
