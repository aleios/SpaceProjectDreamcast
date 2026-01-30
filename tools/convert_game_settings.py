import json
import sys
import helpers

def write_playlist(output_file, playlist):
    helpers.write_ushort(output_file, len(playlist))
    for entry in playlist:
        helpers.write_str(output_file, entry)

def parse_settings(input_file, output_fname):
    settings = json.load(input_file)

    with open(output_fname, "wb") as output_file:
        helpers.write_ushort(output_file, settings.get("max_lives", 3))
        helpers.write_ushort(output_file, settings.get("max_health", 10))
        write_playlist(output_file, settings.get("playlist", []))

def main():
    input_fname = sys.argv[-2]
    output_fname = sys.argv[-1]

    print("[GameSettings Convert] ", input_fname)
    with open(input_fname, "r") as input_file:
        parse_settings(input_file, output_fname)

main()