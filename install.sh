#!/bin/bash
# Installation script for Pdembedded project

echo "=========================================="
echo "Pdembedded - Installation Script"
echo "=========================================="
echo ""

# Check if Python 3 is installed
if ! command -v python3 &> /dev/null; then
    echo "ERROR: Python 3 is not installed. Please install Python 3.8 or higher."
    exit 1
fi

echo "✓ Python 3 found: $(python3 --version)"
echo ""

# Create virtual environment (optional but recommended)
echo "Creating Python virtual environment..."
python3 -m venv venv
source venv/bin/activate

echo "✓ Virtual environment activated"
echo ""

# Install Python dependencies
echo "Installing Python dependencies from requirements.txt..."
pip install --upgrade pip
pip install -r requirements.txt

echo "✓ Python dependencies installed"
echo ""

# Check for system dependencies
echo "Checking for system dependencies..."
echo ""

if ! command -v hvcc &> /dev/null; then
    echo "⚠ WARNING: 'hvcc' (Heavy Data compiler) not found."
    echo "  Install it with: pip install hvcc"
    echo ""
fi

if ! command -v idf.py &> /dev/null; then
    echo "⚠ WARNING: 'idf.py' (ESP-IDF) not found."
    echo "  Follow ESP-IDF setup guide: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html"
    echo ""
fi

echo "=========================================="
echo "Installation complete!"
echo ""
echo "To run the GUI:"
echo "  source venv/bin/activate  # (if using venv)"
echo "  python gui.py"
echo "=========================================="
