import json
import sys
import helpers

def parse_collectable(input_file, output_fname):
    settings = json.load(input_file)

    with open(output_fname, "wb") as output_file:
        anim = settings.get('animation', '')
        key = settings.get('animation_key', '')

        if not anim:
            print("[Collectable Convert] Warning: Missing animation name.")
        if not key:
            print("[Collectable Convert] Warning: Missing animation clip.")

        helpers.write_str(output_file, anim)
        helpers.write_str(output_file, key)
        helpers.write_float(output_file, settings.get('lifetime', 0.0))
        helpers.write_str(output_file, settings.get('pickup_sound', ''))
        helpers.write_float(output_file, settings.get('collider_radius', 0.0))
        helpers.write_float(output_file, settings.get('speed', 0.1))
        helpers.write_byte(output_file, settings.get('health', 0))
        helpers.write_byte(output_file, settings.get('lives', 0))
        helpers.write_byte(output_file, settings.get('weapon', 0))
        helpers.write_short(output_file, settings.get('score', 0))

def main():
    input_fname = sys.argv[-2]
    output_fname = sys.argv[-1]

    print("[Collectable Convert] ", input_fname)
    with open(input_fname, "r") as input_file:
        parse_collectable(input_file, output_fname)

main()