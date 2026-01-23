import json
import sys
import helpers

def write_weapons(output_file, weapons):
    helpers.write_ushort(output_file, len(weapons))
    print("Weapons ", weapons)
    for weap in weapons:
        emitters = weap.get("emitters", [])
        helpers.write_ushort(output_file, len(emitters))
        for emitter in emitters:
            helpers.write_emitter(output_file, emitter)

def write_playlist(output_file, playlist):
    helpers.write_ushort(output_file, len(playlist))
    for entry in playlist:
        helpers.write_str(output_file, entry)

def parse_settings(input_file, output_fname):
    settings = json.load(input_file)

    weapons = settings.get("weapons", [])

    with open(output_fname, "wb") as output_file:
        helpers.write_ushort(output_file, settings.get("max_lives", 3))
        helpers.write_ushort(output_file, settings.get("max_health", 10))
        helpers.write_float(output_file, settings.get("player_speed", 0.2))
        write_weapons(output_file, weapons)
        write_playlist(output_file, settings.get("playlist", []))

def main():
    input_fname = sys.argv[-2]
    output_fname = sys.argv[-1]

    print("[GameSettings Convert] ", input_fname)
    with open(input_fname, "r") as input_file:
        parse_settings(input_file, output_fname)

main()