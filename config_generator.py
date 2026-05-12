#!/usr/bin/env python3
"""
I2S Configuration Generator
Generates C++ header file with I2S pin configuration from JSON
"""

import jinja2
import json
import os
from datetime import datetime
from pathlib import Path


class I2SConfigGenerator:
    """Generate I2S configuration header from JSON settings"""
    
    def __init__(self, template_dir: str):
        """
        Initialize the generator with a template directory
        
        Args:
            template_dir: Path to directory containing Jinja2 templates
        """
        self.template_dir = template_dir
        self.jinja_env = jinja2.Environment(
            loader=jinja2.FileSystemLoader(template_dir),
            trim_blocks=True,
            lstrip_blocks=True
        )
    
    def generate_from_dict(self, config: dict) -> str:
        """
        Generate I2S config header from dictionary
        
        Args:
            config: Dictionary with keys: bclk_pin, ws_pin, dout_pin
            
        Returns:
            Rendered C++ header file content as string
        """
        template = self.jinja_env.get_template('i2s_config.h.j2')
        
        # Add timestamp to config
        config['timestamp'] = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        
        return template.render(**config)
    
    def generate_from_file(self, json_file: str) -> str:
        """
        Generate I2S config header from JSON file
        
        Args:
            json_file: Path to JSON configuration file
            
        Returns:
            Rendered C++ header file content as string
        """
        with open(json_file, 'r', encoding='utf-8') as f:
            config = json.load(f)
        
        return self.generate_from_dict(config)
    
    def write_config(self, config: dict, output_file: str) -> bool:
        """
        Generate and write I2S config header to file
        
        Args:
            config: Dictionary with I2S configuration
            output_file: Path where to write the generated header
            
        Returns:
            True if successful, False otherwise
        """
        try:
            header_content = self.generate_from_dict(config)
            
            # Ensure output directory exists
            output_path = Path(output_file)
            output_path.parent.mkdir(parents=True, exist_ok=True)
            
            with open(output_file, 'w', encoding='utf-8') as f:
                f.write(header_content)
            
            return True
        except Exception as e:
            print(f"Error writing I2S config: {e}")
            return False


def create_default_config() -> dict:
    """
    Create a default I2S configuration
    
    Returns:
        Dictionary with default I2S settings
    """
    return {
        'bclk_pin': 27,
        'ws_pin': 26,
        'dout_pin': 25
    }


if __name__ == '__main__':
    import sys
    
    if len(sys.argv) < 2:
        print("Usage: python config_generator.py <json_config_file> [output_file]")
        print("\nExample:")
        print("  python config_generator.py .i2s_config.json build/config/i2s_config.h")
        sys.exit(1)
    
    json_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else "i2s_config.h"
    
    # Get template directory (same directory as this script)
    template_dir = os.path.join(os.path.dirname(__file__), 'templates')
    
    generator = I2SConfigGenerator(template_dir)
    
    if generator.write_config(
        json.load(open(json_file)),
        output_file
    ):
        print(f"✓ Generated I2S config: {output_file}")
    else:
        print(f"✗ Failed to generate I2S config")
        sys.exit(1)
