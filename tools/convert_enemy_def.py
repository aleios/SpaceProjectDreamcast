import sys
import json
import struct
import helpers
import math

def parse_proj_def(input_file, output_fname):
    data = json.load(input_file)

    animation = data.get('animation', '')
    idle_key = data.get('idle_key', '')
    left_key = data.get('left_key', '')
    right_key = data.get('right_key', '')

    health = data.get('health', 1)
    collision_radius = data.get('collision_radius', 1.0)
    score = data.get('score', 1)

    script = data.get('script', '')

    with open(output_fname, "wb") as output_file:
        helpers.write_magicnum(output_file, 'EDEF')
        helpers.write_str(output_file, animation)
        helpers.write_str(output_file, idle_key)
        helpers.write_str(output_file, left_key)
        helpers.write_str(output_file, right_key)

        helpers.write_ushort(output_file, health)
        helpers.write_float(output_file, collision_radius)
        helpers.write_short(output_file, score)

        helpers.write_str(output_file, script)


def main():
    input_fname = sys.argv[-2]
    output_fname = sys.argv[-1]

    print("[Enemy Def] ", input_fname)
    with open(input_fname, "r") as input_file:
        parse_proj_def(input_file, output_fname)

main()