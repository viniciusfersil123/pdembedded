#!/usr/bin/env python3
"""
Minimal CustomTkinter GUI for ESP32 project management
"""

import customtkinter as ctk
import subprocess
import os

ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")


class EmbeddedProjectGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("ESP32 Manager")
        self.root.geometry("300x150")
        
        self.project_path = os.path.dirname(os.path.abspath(__file__))
        self.setup_ui()
    
    def setup_ui(self):
        """Setup minimal UI"""
        
        # Header
        header = ctk.CTkLabel(
            self.root,
            text="ESP32 Project Manager",
            font=ctk.CTkFont(size=14, weight="bold")
        )
        header.pack(pady=10)
        
        # Buttons frame
        button_frame = ctk.CTkFrame(self.root)
        button_frame.pack(pady=10, padx=10, fill="x")
        
        self.build_btn = ctk.CTkButton(
            button_frame,
            text="Build",
            command=self.run_build,
            height=40,
            font=ctk.CTkFont(size=12)
        )
        self.build_btn.grid(row=0, column=0, padx=5, pady=5, sticky="ew")
        
        self.flash_btn = ctk.CTkButton(
            button_frame,
            text="Flash",
            command=self.run_flash,
            height=40,
            font=ctk.CTkFont(size=12)
        )
        self.flash_btn.grid(row=0, column=1, padx=5, pady=5, sticky="ew")
        
        button_frame.grid_columnconfigure(0, weight=1)
        button_frame.grid_columnconfigure(1, weight=1)
    
    def run_build(self):
        """Run idf.py build"""
        subprocess.Popen(
            "idf.py build",
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
