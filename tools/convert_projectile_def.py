import sys
import json
import struct
import helpers

def parse_proj_def(input_file, output_fname):
    data = json.load(input_file)

    texture = data.get("texture", "")
    animation = data.get("animation", "")
    animation_key = data.get("animation_key", "")
    damage = data.get("damage", 1)
    collider_radius = data.get("collider_radius", 1.0)
    rotates = data.get("rotates", False)
    
    with open(output_fname, "wb") as output_file:
        helpers.write_magicnum(output_file, 'PDEF')
        helpers.write_str(output_file, texture)
        helpers.write_str(output_file, animation)
        helpers.write_str(output_file, animation_key)
        helpers.write_ushort(output_file, damage)
        helpers.write_float(output_file, collider_radius)
        helpers.write_ushort(output_file, rotates)


def main():
    input_fname = sys.argv[-2]
    output_fname = sys.argv[-1]

    print("[Projectile Def] ", input_fname)
    with open(input_fname, "r") as input_file:
        parse_proj_def(input_file, output_fname)

main()