import sys
import json
import helpers

def parse_font(input_file, output_fname):
    data = json.load(input_file)

    with open(output_fname, "wb") as output_file:
        helpers.write_str(output_file, data.get("texture", ""))
        helpers.write_ushort(output_file, data.get("cell_width", 16))
        helpers.write_ushort(output_file, data.get("cell_height", 16))

def main():
    input_fname = sys.argv[-2]
    output_fname = sys.argv[-1]

    print("[Font Convert] ", input_fname)
    with open(input_fname, "r") as input_file:
        parse_font(input_file, output_fname)

main()