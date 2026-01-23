import sys
from PIL import Image
import argparse

def convert_to_c_array(args):
    image_path = args.input_filename
    try:
        img = Image.open(image_path)
    except Exception as e:
        print(f"Error opening image: {e}")
        return

    # Ensure image is 48x32
    # Just make it 48x32 in the first place though or you will hate your life.
    if img.size != (48, 32):
        img = img.resize((48, 32), Image.LANCZOS)

    # Convert to grayscale
    img = img.convert('L')
    pixels = img.load()

    data = []
    for y in range(32):
        for x_byte in range(0, 48, 8):
            byte = 0
            for bit in range(8):
                x = x_byte + bit

                # Rotate
                if args.rotate:
                    px = pixels[47 - x, 31 - y]
                else:
                    px = pixels[x, y]

                if args.invert:
                    if px <= 128:
                        byte |= (0x80 >> bit)
                else:
                    if px > 128:
                        byte |= (0x80 >> bit)
            data.append(byte)

    base_name = image_path.split('/')[-1].split('.')[0]
    var_name = "".join([c if c.isalnum() else "_" for c in base_name])

    # Output the C array.
    if args.output is None:
        args.output = f"{base_name}.c"

    with open(args.output, "w") as output_file:
        print(f"static uint8_t {var_name}_vmu_icon[192] = {{", file=output_file)
        for i, b in enumerate(data):
            if i % 12 == 0:
                print("    ", end="", file=output_file)
            print(f"0x{b:02x}", end="", file=output_file)
            if i < len(data) - 1:
                print(", ", end="", file=output_file)
            if (i + 1) % 12 == 0:
                print(file=output_file)
        print("};", file=output_file)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog="icongen",
        description="Generate an icon for use with the VMU"
    )
    parser.add_argument("input_filename", type=str, help="Path to the source image")
    parser.add_argument("-o", "--output", type=str, help="Output for C array source")
    parser.add_argument("--invert", help="Invert which pixels are 'on'", action="store_true")
    parser.add_argument("--no-rotate", help="Do not rotate bottom-right to top-left", dest="rotate", action="store_false")

    args = parser.parse_args()

    if len(sys.argv) < 2 or args.input_filename is None:
        #print("Usage: python main.py <image_path>")
        parser.print_help()
    else:
        convert_to_c_array(args)
