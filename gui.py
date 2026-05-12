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
        
        self.build_flash_btn = ctk.CTkButton(
            button_frame,
            text="Build and Flash",
            command=self.run_build_and_flash,
            height=40,
            font=ctk.CTkFont(size=12)
        )
        self.build_flash_btn.pack(padx=5, pady=5, fill="x")
    
    def run_build_and_flash(self):
        """Run build and flash sequentially"""
        subprocess.Popen(
            "rm -rf ./main/output && hvcc ./main/test.pd -o .main/output && idf.py build && idf.py flash",
            cwd=self.project_path,
            shell=True
        )
    
    def run_build(self):
        """Run idf.py build"""
        subprocess.Popen(
            "rm -rf ./main/output && hvcc ./main/test.pd -o .main/output && idf.py build",
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
