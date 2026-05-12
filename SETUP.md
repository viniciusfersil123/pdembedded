# Pdembedded - Setup & Installation

## Quick Start

### Option 1: Automated Installation (Linux/macOS)
```bash
chmod +x install.sh
./install.sh
```

### Option 2: Manual Installation

#### Step 1: Install Python Dependencies
```bash
# (Optional but recommended) Create virtual environment
python3 -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install Python dependencies
pip install --upgrade pip
pip install -r requirements.txt
```

#### Step 2: Install ESP-IDF (idf.py)

**Critical:** ESP-IDF must be installed separately from the system, not via pip.

Follow the official installation guide:
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html

Quick reference for Linux/macOS:
```bash
# Clone ESP-IDF repository
git clone https://github.com/espressif/esp-idf.git ~/esp/esp-idf
cd ~/esp/esp-idf

# Install tools
./install.sh

# Activate ESP-IDF environment (do this before running the GUI)
source ~/esp/esp-idf/export.sh
```

**Windows:** Use the ESP-IDF Installer from the official site.

#### Step 3: Set Target Chip
```bash
cd /path/to/pdembedded
idf.py set-target esp32
```

#### Step 4: Run the GUI
```bash
# Make sure ESP-IDF environment is activated (Linux/macOS)
source ~/esp/esp-idf/export.sh

python gui.py
```

## Requirements

- Python 3.8+
- customtkinter (installed via pip)
- hvcc (Pure Data compiler, installed via pip)
- ESP-IDF tools (installed separately, not via pip)
- tkinter (usually comes with Python)

## Environment Variables (Optional but Recommended)

Add these to your shell profile (~/.bashrc, ~/.zshrc, etc.):
```bash
export PATH=$HOME/esp/esp-idf/tools:$PATH
export IDF_PATH=$HOME/esp/esp-idf
```

Then activate on demand:
```bash
source $IDF_PATH/export.sh
```

## Troubleshooting

### "ModuleNotFoundError: No module named 'customtkinter'"
```bash
pip install customtkinter
```

### "hvcc command not found"
```bash
pip install hvcc
```

### "idf.py command not found"
The ESP-IDF environment is not activated. Run:
```bash
source ~/esp/esp-idf/export.sh
```
Or follow the full installation guide: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html

### "idf.py: CMakeLists.txt file not found"
Make sure you're running the GUI from the project directory where CMakeLists.txt exists.
