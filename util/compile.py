#!/usr/bin/env python3
import sys
import os
import struct
import re

def compile_font(input_file, output_file):
    """Compile .ft font source to .bin font file"""
    
    font_data = {
        'name': 'Unknown',
        'charw': 8,
        'charh': 16,
        'first': 32,
        'last': 127,
        'bpp': 1,
        'bitmap': []
    }
    
    # Parse source file
    with open(input_file, 'r') as f:
        content = f.read()
    
    # Extract directives using regex
    name_match = re.search(r'FONTNAME\s+"([^"]+)"', content)
    if name_match:
        font_data['name'] = name_match.group(1)
    
    charw_match = re.search(r'CHARW\s+(\d+)', content)
    if charw_match:
        font_data['charw'] = int(charw_match.group(1))
    
    charh_match = re.search(r'CHARH\s+(\d+)', content)
    if charh_match:
        font_data['charh'] = int(charh_match.group(1))
    
    first_match = re.search(r'FIRST\s+(\d+)', content)
    if first_match:
        font_data['first'] = int(first_match.group(1))
    
    last_match = re.search(r'LAST\s+(\d+)', content)
    if last_match:
        font_data['last'] = int(last_match.group(1))
    
    bpp_match = re.search(r'BPP\s+(\d+)', content)
    if bpp_match:
        font_data['bpp'] = int(bpp_match.group(1))
    
    # Extract bitmap data - everything between FONTDATA = [ and ]
    data_match = re.search(r'FONTDATA\s*=\s*\[\s*(.*?)\s*\]', content, re.DOTALL)
    if data_match:
        data_section = data_match.group(1)
        
        # Split by commas and clean up
        for item in data_section.split(','):
            item = item.strip()
            if not item:
                continue
            
            # Handle hex values (0xXX or just XX)
            if item.startswith('0x') or item.startswith('0X'):
                try:
                    val = int(item, 16)
                    font_data['bitmap'].append(val)
                except ValueError:
                    print(f"Warning: Could not parse '{item}' as hex")
            else:
                # Try as decimal? Probably hex without 0x prefix
                try:
                    val = int(item, 16)
                    font_data['bitmap'].append(val)
                except ValueError:
                    try:
                        val = int(item)
                        font_data['bitmap'].append(val)
                    except ValueError:
                        print(f"Warning: Could not parse '{item}'")
    
    # Calculate metadata
    chars = font_data['last'] - font_data['first'] + 1
    pixels_per_byte = 8 // font_data['bpp']
    bytes_per_row = (font_data['charw'] + pixels_per_byte - 1) // pixels_per_byte
    expected_size = chars * bytes_per_row * font_data['charh']
    
    print(f"Compiling font: {font_data['name']}")
    print(f"  Size: {font_data['charw']}x{font_data['charh']}")
    print(f"  Range: {font_data['first']}-{font_data['last']} ({chars} chars)")
    print(f"  BPP: {font_data['bpp']}")
    print(f"  Bitmap data: {len(font_data['bitmap'])} bytes")
    print(f"  Expected: {expected_size} bytes")
    
    if len(font_data['bitmap']) != expected_size:
        print(f"  WARNING: Bitmap size mismatch!")
        print(f"  This might be due to missing or extra data in the font file.")
    
    if len(font_data['bitmap']) == 0:
        print(f"  ERROR: No bitmap data found! Check the FONTDATA section format.")
        return
    
    # Write binary font file
    with open(output_file, 'wb') as f:
        # Header
        f.write(b'FONT')                 # Magic
        f.write(struct.pack('<I', 1))     # Version
        f.write(struct.pack('<I', font_data['charw']))
        f.write(struct.pack('<I', font_data['charh']))
        f.write(struct.pack('<I', font_data['first']))
        f.write(struct.pack('<I', font_data['last']))
        f.write(struct.pack('<I', font_data['bpp']))
        f.write(struct.pack('<I', len(font_data['bitmap'])))
        
        # Name (null-terminated)
        name_bytes = font_data['name'].encode('utf-8')
        f.write(name_bytes)
        f.write(b'\x00')
        
        # Bitmap data
        f.write(bytes(font_data['bitmap']))
    
    print(f"Wrote {output_file} ({os.path.getsize(output_file)} bytes)")

def main():
    if len(sys.argv) != 3:
        print("Usage: fontc.py <input.ft> <output.bin>")
        print("Example: fontc.py cascadia.ft cascadia.bin")
        return
    
    if not os.path.exists(sys.argv[1]):
        print(f"Error: Input file '{sys.argv[1]}' not found")
        return
    
    compile_font(sys.argv[1], sys.argv[2])

if __name__ == "__main__":
    main()