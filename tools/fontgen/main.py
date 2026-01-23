import math
import os
import argparse
from PIL import Image, ImageFont, ImageDraw

def next_power_of_2(n):
    return 1 if n <= 0 else 2**(n - 1).bit_length()

def generate_bitmap_font(font_path, font_size, cell_size, char_list, output_path):
    num_chars = len(char_list)
    cell_w, cell_h = cell_size
    
    # Find smallest POT resolution
    total_area = num_chars * cell_w * cell_h
    side = next_power_of_2(int(math.sqrt(total_area)))
    
    while (side // cell_w) * (side // cell_h) < num_chars:
        side *= 2
    
    sheet_w, sheet_h = side, side
    cols = sheet_w // cell_w

    image = Image.new("RGBA", (sheet_w, sheet_h), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)

    # Turn off the anti-aliasing.
    draw.fontmode = "1"

    try:
        font = ImageFont.truetype(font_path, font_size)
    except Exception as e:
        print(f"Error: Could not load font file '{font_path}': {e}")
        return

    ascent, descent = font.getmetrics()
    line_height = ascent + descent
    vertical_margin = (cell_h - line_height) // 2

    for index, char in enumerate(char_list):
        col = index % cols
        row = index // cols
        
        x_offset = col * cell_w
        y_offset = row * cell_h
        
        bbox = draw.textbbox((0, 0), char, font=font)
        char_w = bbox[2] - bbox[0]

        # Center horizontally within the cell, adjust for internal font bbox offset
        x_draw = x_offset + (cell_w - char_w) // 2 - bbox[0]
        y_draw = y_offset + vertical_margin
        
        draw.text((x_draw, y_draw), char, font=font, fill=(255, 255, 255, 255))

    try:
        image.save(output_path)
        print(f"Successfully generated bitmap font:")
        print(f"  Texture: {sheet_w}x{sheet_h}")
        print(f"  Grid:    {cols} columns, Cell size {cell_w}x{cell_h}")
        print(f"  Output:  {output_path}")
    except Exception as e:
        print(f"Error: Could not save output image: {e}")

def main():
    parser = argparse.ArgumentParser(
        prog="fontgen",
        description="Generate a Power-of-Two bitmap font sheet from a TTF/OTF file."
    )
    parser.add_argument("input_filename", help="Path to the source font file")
    parser.add_argument("-o", "--output", help="Output PNG path (defaults to input name + .png)")
    parser.add_argument("-s", "--size", type=int, default=16, help="Font size (default: 16)")
    parser.add_argument("-p", "--padding", type=int, default=0, help="Extra padding around each cell (default: 0)")
    parser.add_argument("--start", type=int, default=32, help="Starting ASCII code (default: 32)")
    parser.add_argument("--end", type=int, default=126, help="Ending ASCII code (default: 126)")
    parser.add_argument('--encoding', type=str, default='ascii', help='Which encoding to use (ascii, shift_jis)')

    args = parser.parse_args()

    # Determine output path if not provided
    output = args.output
    if not output:
        output = os.path.splitext(args.input_filename)[0] + '.png'

    # Ensure input exists
    if not os.path.exists(args.input_filename):
        print(f"Error: Input file '{args.input_filename}' not found.")
        return
    
    # Char list for encoding
    chars = []
    if args.encoding.lower() == 'ascii':
        chars = [chr(i) for i in range(args.start, args.end + 1)]
    elif args.encoding.lower() in ['shift_jis', 'sjis']:

        # 1-byte range (ASCII + Katakana)
        for i in range(0x20, 0xE0):
            try:
                char = bytes([i]).decode('shift_jis')
                chars.append(char)
            except UnicodeDecodeError:
                continue

        # 2-byte range for hiragana/kanji
        # TODO: What the fuck am I even doing here...
        lead_ranges = list(range(0x81, 0x85)) + list(range(0x88, 0x99))
        for b1 in lead_ranges:
            for b2 in range(0x40, 0xFD):
                if b2 == 0x7F: continue
                try:
                    char = bytes([b1, b2]).decode("shift_jis")
                    chars.append(char)
                except UnicodeDecodeError:
                    continue
    else:
        print(f"{args.encoding} encoding not suported.")
        return

    generate_bitmap_font(
        font_path=args.input_filename,
        font_size=args.size, 
        cell_size=(args.size + args.padding, args.size + args.padding),
        # range=(args.start, args.end), 
        char_list=chars,
        output_path=output
    )

main()