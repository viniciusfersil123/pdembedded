#!/usr/bin/env python3
"""
Minimal CustomTkinter GUI for ESP32 project management
"""

import customtkinter as ctk
import subprocess
import os
import threading
import shlex
from tkinter import filedialog

ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")


class EmbeddedProjectGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Pdembedded")
        self.root.geometry("700x420")
        
        self.project_path = os.path.dirname(os.path.abspath(__file__))
        self._selected_patch_store = os.path.join(self.project_path, ".selected_patch")
        self.patch_path = self.load_selected_patch()
        self.current_process = None
        self.setup_ui()
    
    def setup_ui(self):
        """Setup minimal UI"""
        
        # Patch selection (appear first)
        patch_frame = ctk.CTkFrame(self.root)
        patch_frame.pack(padx=10, pady=(10, 8), fill="x")

        self.patch_file_label = ctk.CTkLabel(patch_frame, text=self.patch_path or "No patch selected")
        self.patch_file_label.pack(side="left", padx=(4, 8), expand=True)

        select_patch_btn = ctk.CTkButton(patch_frame, text="Select Patch", command=self.select_patch, height=30)
        select_patch_btn.pack(side="right", padx=4)

        # Terminal output area (middle)
        self.log_box = ctk.CTkTextbox(self.root, height=260)
        self.log_box.pack(padx=10, pady=(0, 10), fill="both", expand=True)
        self.log_box.insert("end", "Ready. Click 'Build and Flash' to start.\n")
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

        self.build_flash_btn.configure(state="disabled")
        self.append_log("\n=== Build and Flash started ===\n")

        patch = self.patch_path or os.path.join(self.project_path, "main", "test.pd")
        patch_quoted = shlex.quote(patch)
        cmd = f"rm -rf ./main/output && hvcc {patch_quoted} -o .main/output && idf.py build && idf.py flash"
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
