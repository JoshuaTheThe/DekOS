#!/usr/bin/env python3
from PIL import Image
import numpy as np
import sys
import os

def convert_bitmap_to_font(image_path, char_width, char_height, bpp, first_char=32):
    """
    Convert a font bitmap with transparency to various BPP formats
    bpp = 1: 1 bit per pixel (monochrome) - 8 pixels per byte
    bpp = 2: 2 bits per pixel (4 levels) - 4 pixels per byte  
    bpp = 4: 4 bits per pixel (16 levels) - 2 pixels per byte
    """
    try:
        # Open image with transparency (RGBA)
        img = Image.open(image_path).convert('RGBA')
        pixels = np.array(img)
        img.show()
        
        print(f"Image size: {img.width}x{img.height}")
        print(f"Character size: {char_width}x{char_height}")
        print(f"Bits per pixel: {bpp}")
        print(f"Image mode: {img.mode}")
        
        # Calculate font layout
        chars_per_row = img.width // char_width
        chars_per_col = img.height // char_height
        total_chars = chars_per_row * chars_per_col
        last_char = first_char + total_chars - 1
        
        print(f"Font layout: {chars_per_row}x{chars_per_col} characters")
        print(f"Character range: {first_char} ('{chr(first_char)}') to {last_char} ('{chr(last_char)}')")
        
        output_data = []
        char_metadata = []
        
        # Calculate pixels per byte based on BPP
        pixels_per_byte = 8 // bpp
        
        for char_index in range(total_chars):
            row = char_index // chars_per_row
            col = char_index % chars_per_row
            
            x_start = col * char_width
            y_start = row * char_height
            
            char_bytes = []
            
            for y in range(char_height):
                if bpp == 1:
                    # 1 BPP: Pack 8 pixels into one uint8_t
                    packed_value = 0
                    for x in range(char_width):
                        # Get RGBA pixel
                        r, g, b, a = pixels[y_start + y, x_start + x]
                        # Use alpha channel to determine visibility
                        # Typically: high alpha = visible (1), low alpha = background (0)
                        bit_value = 1 if a > 128 else 0
                        # Pack into byte (MSB first)
                        packed_value |= (bit_value << (7 - (x % 8)))
                        
                        # When we have 8 pixels or reach end of row, store the byte
                        if (x + 1) % 8 == 0 or x == char_width - 1:
                            char_bytes.append(packed_value)
                            packed_value = 0
                    
                elif bpp == 2:
                    # 2 BPP: Pack 4 pixels into one uint8_t
                    packed_value = 0
                    pixels_in_byte = 0
                    
                    for x in range(char_width):
                        # Get RGBA pixel
                        r, g, b, a = pixels[y_start + y, x_start + x]
                        # Convert alpha to 2-bit value (0-3)
                        # Map alpha range (0-255) to 2-bit range (0-3)
                        bit_value = min(3, a // 64)  # 0-63=0, 64-127=1, 128-191=2, 192-255=3
                        
                        # Pack into byte (2 bits per pixel, MSB first)
                        packed_value |= (bit_value << (6 - (pixels_in_byte * 2)))
                        pixels_in_byte += 1
                        
                        # When we have 4 pixels or reach end of row, store the byte
                        if pixels_in_byte == 4 or x == char_width - 1:
                            char_bytes.append(packed_value)
                            packed_value = 0
                            pixels_in_byte = 0
                            
                elif bpp == 4:
                    # 4 BPP: Pack 2 pixels into one uint8_t
                    packed_value = 0
                    pixels_in_byte = 0
                    
                    for x in range(char_width):
                        # Get RGBA pixel
                        r, g, b, a = pixels[y_start + y, x_start + x]
                        # Convert alpha to 4-bit value (0-15)
                        bit_value = a // 16  # Map 0-255 to 0-15
                        
                        # Pack into byte (4 bits per pixel, MSB first)
                        packed_value |= (bit_value << (4 - (pixels_in_byte * 4)))
                        pixels_in_byte += 1
                        
                        # When we have 2 pixels or reach end of row, store the byte
                        if pixels_in_byte == 2 or x == char_width - 1:
                            char_bytes.append(packed_value)
                            packed_value = 0
                            pixels_in_byte = 0
            
            output_data.extend(char_bytes)
            
            # Store character metadata for debugging
            ascii_code = first_char + char_index
            char_metadata.append({
                'ascii': ascii_code,
                'char': chr(ascii_code) if 32 <= ascii_code <= 126 else '?',
                'position': (col, row),
                'bytes': char_bytes,
                'bytes_per_row': len(char_bytes) // char_height
            })
        
        return output_data, total_chars, last_char, char_metadata
        
    except Exception as e:
        print(f"Error converting font: {e}")
        import traceback
        traceback.print_exc()
        return None, 0, 0, []

def generate_c_code(bitmap_data, font_name, char_width, char_height, first_char, last_char, bpp, metadata):
    """Generate C source code from the converted font data"""
    
    # Calculate bytes per character and bytes per row
    pixels_per_byte = 8 // bpp
    bytes_per_row = (char_width + pixels_per_byte - 1) // pixels_per_byte
    bytes_per_char = bytes_per_row * char_height
    
    c_code = f"""#include "fonts.h"
#include <stdint.h>

/* Converted {font_name} font - {char_width}x{char_height}, {bpp}bpp */
/* Characters: {first_char} ('{chr(first_char) if first_char >= 32 else ' '}') to {last_char} ('{chr(last_char) if last_char >= 32 else ' '}') */
/* Total data: {len(bitmap_data)} bytes, {bytes_per_char} bytes per character */
/* Packing: {pixels_per_byte} pixels per byte */
/* Source: Transparency-based font bitmap */

static uint8_t {font_name.lower()}Bitmap[] = {{
"""
    
    # Format the data with 16 values per line for readability
    for i in range(0, len(bitmap_data), 16):
        line = "    " + ", ".join(f"0x{val:02X}" for val in bitmap_data[i:i+16])
        c_code += line + ",\n"
    
    # Remove trailing comma if present
    if c_code.endswith(",\n"):
        c_code = c_code[:-2] + "\n"
    
    c_code += """};

font_t timesNewRoman = {
    .font_name = \"""" + font_name + """\",
    .font_bitmap = """ + font_name.lower() + """Bitmap,
    .char_width = """ + str(char_width) + """,
    .char_height = """ + str(char_height) + """,
    .first_char = """ + str(first_char) + """,
    .last_char = """ + str(last_char) + """,
    .bpp = """ + str(bpp) + """
};

"""

    # Add character map for reference
    c_code += "/* Character map:\n"
    chars_per_line = 16
    for i in range(0, len(metadata), chars_per_line):
        line_chars = metadata[i:i+chars_per_line]
        line = "   "
        for meta in line_chars:
            if meta['ascii'] >= 32 and meta['ascii'] <= 126:
                line += f" {meta['ascii']:3d}('{meta['char']}')"
        c_code += line + "\n"
    c_code += "*/\n"
    
    return c_code

def print_character_preview(metadata, char_width, char_height, bpp, first_char, sample_chars="ABC123"):
    """Print a visual preview of sample characters"""
    print(f"\nCharacter preview for: {sample_chars}")
    
    # Calculate bytes per row
    pixels_per_byte = 8 // bpp
    bytes_per_row = (char_width + pixels_per_byte - 1) // pixels_per_byte
    
    for sample_char in sample_chars:
        ascii_code = ord(sample_char)
        if ascii_code < first_char or ascii_code >= first_char + len(metadata):
            continue
            
        char_index = ascii_code - first_char
        char_data = metadata[char_index]['bytes']
        
        print(f"\n'{sample_char}' (ASCII {ascii_code}):")
        
        for row in range(char_height):
            row_start = row * bytes_per_row
            row_bytes = char_data[row_start:row_start + bytes_per_row]
            
            # Reconstruct the pixel row for display
            pixel_row = ""
            if bpp == 1:
                for byte_val in row_bytes:
                    for bit in range(7, -1, -1):
                        if len(pixel_row) < char_width:
                            pixel_val = (byte_val >> bit) & 1
                            pixel_row += "*" if pixel_val else " "
            elif bpp == 2:
                for byte_val in row_bytes:
                    for pixel_pair in range(3, -1, -1):
                        if len(pixel_row) < char_width:
                            pixel_val = (byte_val >> (pixel_pair * 2)) & 0x03
                            # Show transparency levels
                            if pixel_val == 0: pixel_row += " "
                            elif pixel_val == 1: pixel_row += "."
                            elif pixel_val == 2: pixel_row += "o"
                            else: pixel_row += "*"
            elif bpp == 4:
                for byte_val in row_bytes:
                    for pixel_pair in range(1, -1, -1):
                        if len(pixel_row) < char_width:
                            pixel_val = (byte_val >> (pixel_pair * 4)) & 0x0F
                            # Show transparency levels
                            if pixel_val == 0: pixel_row += " "
                            elif pixel_val < 4: pixel_row += "."
                            elif pixel_val < 8: pixel_row += "o"
                            elif pixel_val < 12: pixel_row += "O"
                            else: pixel_row += "*"
            
            print(f"  {pixel_row}")

def main():
    if len(sys.argv) not in [5, 6]:
        print("Usage: python convert_font.py <image_file> <char_width> <char_height> <bpp> [first_char]")
        print("  bpp: 1 (monochrome), 2 (4 levels), or 4 (16 levels)")
        print("Example: python convert_font.py times_new_roman.bmp 8 12 2 32")
        return
    
    image_file = sys.argv[1]
    char_width = int(sys.argv[2])
    char_height = int(sys.argv[3])
    bpp = int(sys.argv[4])
    first_char = int(sys.argv[5]) if len(sys.argv) > 5 else 32
    
    if bpp not in [1, 2, 4]:
        print("Error: bpp must be 1, 2, or 4")
        return
    
    if not os.path.exists(image_file):
        print(f"Error: File '{image_file}' not found")
        return
    
    print(f"Converting {image_file} to {bpp}bpp font (using transparency)...")
    bitmap_data, total_chars, last_char, metadata = convert_bitmap_to_font(
        image_file, char_width, char_height, bpp, first_char
    )
    
    if bitmap_data is None:
        return
    
    print(f"Generated {len(bitmap_data)} uint8_t bytes ({total_chars} characters)")
    
    # Generate C code
    c_code = generate_c_code(bitmap_data, "TimesNewRoman", char_width, char_height, 
                           first_char, last_char, bpp, metadata)
    
    # Write to file
    output_file = "converted_font.c"
    with open(output_file, "w") as f:
        f.write(c_code)
    
    print(f"Conversion complete! Output written to {output_file}")
    
    # Print preview of some characters
    print_character_preview(metadata, char_width, char_height, bpp, first_char)

if __name__ == "__main__":
    main()