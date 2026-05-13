#!/usr/bin/env python3
"""
Minimal CustomTkinter GUI for ESP32 project management
"""

# Attempt to activate a local virtualenv early so top-level imports use it
import atexit
import os
_venv_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "venv")
_venv_bin = os.path.join(_venv_path, "bin")
_venv_old_path = None
_venv_old_virtual_env = None
_venv_activated = False
try:
    if os.path.isdir(_venv_bin):
        _venv_old_path = os.environ.get("PATH")
        _venv_old_virtual_env = os.environ.get("VIRTUAL_ENV")
        os.environ["PATH"] = _venv_bin + os.pathsep + (_venv_old_path or "")
        os.environ["VIRTUAL_ENV"] = _venv_path
        _venv_activated = True
except Exception:
    _venv_activated = False

def _deactivate_module_venv():
    global _venv_activated
    try:
        if not _venv_activated:
            return
        if _venv_old_path is not None:
            os.environ["PATH"] = _venv_old_path
        else:
            parts = [p for p in os.environ.get("PATH", "").split(os.pathsep) if p != _venv_bin]
            os.environ["PATH"] = os.pathsep.join(parts)
        if _venv_old_virtual_env is not None:
            os.environ["VIRTUAL_ENV"] = _venv_old_virtual_env
        else:
            os.environ.pop("VIRTUAL_ENV", None)
        _venv_activated = False
    except Exception:
        pass

atexit.register(_deactivate_module_venv)

import sys
import site as _site

# If the venv contains site-packages, add it to this interpreter's sys.path
try:
    _venv_site = os.path.join(_venv_path, "lib", f"python{sys.version_info.major}.{sys.version_info.minor}", "site-packages")
    if os.path.isdir(_venv_site):
        _site.addsitedir(_venv_site)
except Exception:
    pass

import customtkinter as ctk
import subprocess
import os
import threading
import shlex
import json
from tkinter import filedialog
from config_generator import I2SConfigGenerator, create_default_config

ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")


class EmbeddedProjectGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Pdembedded")
        self.root.geometry("800x650")
        
        self.project_path = os.path.dirname(os.path.abspath(__file__))
        self._selected_patch_store = os.path.join(self.project_path, ".selected_patch")
        self._i2s_config_store = os.path.join(self.project_path, ".i2s_config.json")
        # Available I2S pins for ESP32 WROOM (avoid strapping pins 0, 2 and flash pins 6-11)
        self.available_pins = [4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33]
        self.selected_pins = {}
        self.pin_combos = {}

        self.patch_path = self.load_selected_patch()
        self.i2s_config = self.load_i2s_config()
        self.current_process = None
        
        # venv handling
        self.venv_path = os.path.join(self.project_path, "venv")
        self._venv_active = False
        self._old_path = None
        self._old_virtual_env = None

        self.setup_ui()

        # Activate project virtualenv (if present) and ensure we deactivate on close
        if os.environ.get("VIRTUAL_ENV") != self.venv_path:
            self.activate_venv()
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)
    
    def setup_ui(self):
        """Setup minimal UI"""
        
        # Patch selection (appear first)
        patch_frame = ctk.CTkFrame(self.root)
        patch_frame.pack(padx=10, pady=(10, 8), fill="x")

        self.patch_file_label = ctk.CTkLabel(patch_frame, text=self.patch_path or "No patch selected")
        self.patch_file_label.pack(side="left", padx=(4, 8), expand=True)

        select_patch_btn = ctk.CTkButton(patch_frame, text="Select Patch", command=self.select_patch, height=30)
        select_patch_btn.pack(side="right", padx=4)

        # I2S Configuration Frame
        i2s_frame = ctk.CTkFrame(self.root)
        i2s_frame.pack(padx=10, pady=(0, 10), fill="x")

        i2s_label = ctk.CTkLabel(i2s_frame, text="I2S Configuration (ESP32 WROOM)", font=ctk.CTkFont(size=11, weight="bold"))
        i2s_label.pack(anchor="w", pady=(5, 8))

        pins_frame = ctk.CTkFrame(i2s_frame)
        pins_frame.pack(fill="x", pady=(0, 8))

        # BCLK Pin
        ctk.CTkLabel(pins_frame, text="BCLK Pin:").grid(row=0, column=0, padx=5, pady=5, sticky="w")
        self.bclk_combo = ctk.CTkComboBox(
            pins_frame, 
            values=[str(p) for p in self.available_pins],
            command=lambda v: self._on_pin_selected("bclk", v),
            width=80,
            state="readonly"
        )
        self.bclk_combo.set(str(self.i2s_config.get('bclk_pin', 27)))
        self.bclk_combo.grid(row=0, column=1, padx=5, pady=5)

        # WS Pin
        ctk.CTkLabel(pins_frame, text="WS Pin:").grid(row=0, column=2, padx=5, pady=5, sticky="w")
        self.ws_combo = ctk.CTkComboBox(
            pins_frame, 
            values=[str(p) for p in self.available_pins],
            command=lambda v: self._on_pin_selected("ws", v),
            width=80,
            state="readonly"
        )
        self.ws_combo.set(str(self.i2s_config.get('ws_pin', 26)))
        self.ws_combo.grid(row=0, column=3, padx=5, pady=5)

        # DOUT Pin
        ctk.CTkLabel(pins_frame, text="DOUT Pin:").grid(row=0, column=4, padx=5, pady=5, sticky="w")
        self.dout_combo = ctk.CTkComboBox(
            pins_frame, 
            values=[str(p) for p in self.available_pins],
            command=lambda v: self._on_pin_selected("dout", v),
            width=80,
            state="readonly"
        )
        self.dout_combo.set(str(self.i2s_config.get('dout_pin', 25)))
        self.dout_combo.grid(row=0, column=5, padx=5, pady=5)

        self.pin_combos = {
            "bclk": self.bclk_combo,
            "ws": self.ws_combo,
            "dout": self.dout_combo,
        }
        self.selected_pins = {
            "bclk": int(self.i2s_config.get('bclk_pin', 27)),
            "ws": int(self.i2s_config.get('ws_pin', 26)),
            "dout": int(self.i2s_config.get('dout_pin', 25)),
        }
        self._update_pin_options()

        # Terminal output area (middle)
        self.log_box = ctk.CTkTextbox(self.root, height=260)
        self.log_box.pack(padx=10, pady=(0, 10), fill="both", expand=True)
        self.log_box.insert("end", "Ready. I2S config will be auto-generated before build.\n")
        self.log_box.configure(state="disabled")

        # Buttons frame (appear after terminal)
        button_frame = ctk.CTkFrame(self.root)
        button_frame.pack(pady=(0, 10), padx=10, fill="x")

        self.build_flash_btn = ctk.CTkButton(
            button_frame,
            text="Build and Flash",
            command=self.run_build_and_flash,
            height=40,
            font=ctk.CTkFont(size=12)
        )
        self.build_flash_btn.pack(padx=5, pady=5, fill="x")

    def append_log(self, text):
        self.log_box.configure(state="normal")
        self.log_box.insert("end", text)
        self.log_box.see("end")
        self.log_box.configure(state="disabled")

    def activate_venv(self):
        """Activate a local virtualenv by updating environment vars for subprocesses."""
        try:
            bin_dir = os.path.join(self.venv_path, "bin")
            if os.path.isdir(bin_dir):
                self._old_path = os.environ.get("PATH")
                self._old_virtual_env = os.environ.get("VIRTUAL_ENV")
                os.environ["PATH"] = bin_dir + os.pathsep + (self._old_path or "")
                os.environ["VIRTUAL_ENV"] = self.venv_path
                self._venv_active = True
                self.append_log(f"Activated virtualenv: {self.venv_path}\n")
            else:
                self.append_log("No virtualenv found at ./venv — continuing without venv.\n")
        except Exception as e:
            self.append_log(f"Error activating venv: {e}\n")

    def deactivate_venv(self):
        """Restore environment to pre-venv state."""
        try:
            if not self._venv_active:
                return
            # restore PATH
            if self._old_path is not None:
                os.environ["PATH"] = self._old_path
            else:
                prefix = os.path.join(self.venv_path, "bin")
                parts = [p for p in os.environ.get("PATH", "").split(os.pathsep) if p != prefix]
                os.environ["PATH"] = os.pathsep.join(parts)

            # restore VIRTUAL_ENV
            if self._old_virtual_env is not None:
                os.environ["VIRTUAL_ENV"] = self._old_virtual_env
            else:
                os.environ.pop("VIRTUAL_ENV", None)

            self._venv_active = False
            self.append_log(f"Deactivated virtualenv: {self.venv_path}\n")
        except Exception as e:
            self.append_log(f"Error deactivating venv: {e}\n")

    def _on_close(self):
        """Called when the GUI is closing: deactivate venv then destroy root."""
        try:
            self.append_log("\nShutting down...\n")
            # If a build/flash process is running, try to terminate it first
            if self.current_process is not None:
                try:
                    self.current_process.terminate()
                except Exception:
                    pass
            self.deactivate_venv()
        finally:
            try:
                self.root.destroy()
            except Exception:
                pass

    def load_selected_patch(self):
        try:
            if os.path.exists(self._selected_patch_store):
                with open(self._selected_patch_store, "r", encoding="utf-8") as f:
                    path = f.read().strip()
                    if path:
                        return path
            # fallback to main/test.pd if present
            default = os.path.join(self.project_path, "main", "test.pd")
            if os.path.exists(default):
                return default
        except Exception:
            pass
        return ""

    def save_selected_patch(self, path):
        try:
            with open(self._selected_patch_store, "w", encoding="utf-8") as f:
                f.write(path or "")
        except Exception:
            pass

    def select_patch(self):
        filetypes = [("Pure Data files", "*.pd"), ("All files", "*")]
        selected = filedialog.askopenfilename(title="Select .pd patch", initialdir=os.path.join(self.project_path, "main"), filetypes=filetypes)
        if selected:
            self.patch_path = selected
            self.save_selected_patch(selected)
            self.patch_file_label.configure(text=self.patch_path)

    def load_i2s_config(self):
        """Load I2S configuration from JSON file or return defaults"""
        try:
            if os.path.exists(self._i2s_config_store):
                with open(self._i2s_config_store, "r", encoding="utf-8") as f:
                    return self._normalize_pin_config(json.load(f))
        except Exception:
            pass
        return self._normalize_pin_config(create_default_config())

    def _normalize_pin_config(self, config):
        defaults = create_default_config()
        normalized = {}
        used_pins = set()

        for key in ("bclk_pin", "ws_pin", "dout_pin"):
            pin = int(config.get(key, defaults[key]))
            if pin not in self.available_pins or pin in used_pins:
                pin = next(candidate for candidate in self.available_pins if candidate not in used_pins)
            normalized[key] = pin
            used_pins.add(pin)

        return normalized

    def save_i2s_config(self):
        """Save current I2S configuration to JSON file"""
        try:
            config = {
                'bclk_pin': int(self.bclk_combo.get()),
                'ws_pin': int(self.ws_combo.get()),
                'dout_pin': int(self.dout_combo.get())
            }
            with open(self._i2s_config_store, "w", encoding="utf-8") as f:
                json.dump(config, f, indent=2)
            self.i2s_config = config
            return True
        except Exception as e:
            self.append_log(f"Error saving I2S config: {e}\n")
            return False

    def _on_pin_selected(self, pin_type, value):
        """Handle pin selection by removing the pin from other dropdowns"""
        if value:
            desired_pin = int(value)
            used_pins = {
                pin
                for other_type, pin in self.selected_pins.items()
                if other_type != pin_type
            }

            if desired_pin in used_pins:
                desired_pin = next(
                    candidate
                    for candidate in self.available_pins
                    if candidate not in used_pins
                )

            self.selected_pins[pin_type] = desired_pin
            self.pin_combos[pin_type].set(str(desired_pin))
            self._update_pin_options()

    def _update_pin_options(self):
        for pin_type, combo in self.pin_combos.items():
            current_pin = self.selected_pins.get(pin_type)
            available_values = [
                str(pin)
                for pin in self.available_pins
                if pin == current_pin or pin not in self.selected_pins.values()
            ]
            combo.configure(values=available_values)
            if current_pin is not None:
                combo.set(str(current_pin))

    def generate_i2s_config(self):
        """Generate I2S configuration header from current settings"""
        if not self.save_i2s_config():
            return

        try:
            template_dir = os.path.join(self.project_path, 'templates')
            output_file = os.path.join(self.project_path, 'build', 'config', 'i2s_config.h')
            
            generator = I2SConfigGenerator(template_dir)
            if generator.write_config(self.i2s_config, output_file):
                self.append_log(f"✓ Generated I2S config: build/config/i2s_config.h\n")
            else:
                self.append_log("✗ Failed to generate I2S config\n")
        except Exception as e:
            self.append_log(f"Error generating I2S config: {e}\n")

    def _run_command_worker(self, cmd):
        try:
            self.current_process = subprocess.Popen(
                cmd,
                cwd=self.project_path,
                shell=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1
            )

            if self.current_process.stdout:
                for line in self.current_process.stdout:
                    self.root.after(0, self.append_log, line)

            return_code = self.current_process.wait()
            self.root.after(0, self.append_log, f"\nProcess finished with exit code {return_code}.\n")
        except Exception as exc:
            self.root.after(0, self.append_log, f"\nError: {exc}\n")
        finally:
            self.current_process = None
            self.root.after(0, lambda: self.build_flash_btn.configure(state="normal"))
    
    def run_build_and_flash(self):
        """Run build and flash sequentially"""
        if self.current_process is not None:
            self.append_log("A process is already running.\n")
            return

        # Generate I2S config first (automatically)
        self.generate_i2s_config()

        self.build_flash_btn.configure(state="disabled")
        self.append_log("\n=== Build and Flash started ===\n")

        patch = self.patch_path or os.path.join(self.project_path, "main", "test.pd")
        patch_quoted = shlex.quote(patch)
        # Source ESP-IDF environment before running build commands
        esp_idf_path = os.path.expanduser("~/esp/esp-idf")
        cmd = f"export IDF_PATH={esp_idf_path} && . $IDF_PATH/export.sh && rm -rf ./main/output && hvcc {patch_quoted} -o ./main/output && idf.py build && idf.py flash"
        threading.Thread(target=self._run_command_worker, args=(cmd,), daemon=True).start()
    
    def run_build(self):
        """Run idf.py build"""
        patch = self.patch_path or os.path.join(self.project_path, "main", "test.pd")
        patch_quoted = shlex.quote(patch)
        subprocess.Popen(
            f"rm -rf ./main/output && hvcc {patch_quoted} -o .main/output && idf.py build",
            cwd=self.project_path,
            shell=True
        )
    
    def run_flash(self):
        """Run idf.py flash"""
        subprocess.Popen(
            "idf.py flash",
            cwd=self.project_path,
            shell=True
        )


if __name__ == "__main__":
    root = ctk.CTk()
    app = EmbeddedProjectGUI(root)
    root.mainloop()
