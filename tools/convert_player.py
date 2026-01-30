import json
import sys
import helpers

def parse_player(input_file, output_fname):
    settings = json.load(input_file)

    weapons = settings.get("weapons", [])

    with open(output_fname, "wb") as output_file:
        helpers.write_str(output_file, settings.get('animation', ''))
        helpers.write_str(output_file, settings.get('idle_clip', ''))
        helpers.write_str(output_file, settings.get('left_clip', ''))
        helpers.write_str(output_file, settings.get('right_clip', ''))
        helpers.write_float(output_file, settings.get("speed", 0.2))
        helpers.write_weapons(output_file, weapons)

def main():
    input_fname = sys.argv[-2]
    output_fname = sys.argv[-1]

    print("[Player Convert] ", input_fname)
    with open(input_fname, "r") as input_file:
        parse_player(input_file, output_fname)

main()