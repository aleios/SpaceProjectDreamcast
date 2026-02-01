import json
import sys
import helpers

def parse_weaponset(input_file, output_fname):
    data = json.load(input_file)

    emitters = data.get("emitters", [])
    if len(emitters) > 10:
        print("Warning: More than 10 emitters. Truncated.")
        emitters = emitters[:10]

    with open(output_fname, "wb") as output_file:
        helpers.write_ubyte(output_file, data.get("mode", 0))
        helpers.write_ushort(output_file, len(emitters))
        for emitter in emitters:
            helpers.write_emitter(output_file, emitter)

def main():
    input_fname = sys.argv[-2]
    output_fname = sys.argv[-1]

    print("[Weaponset Def Convert] ", input_fname)
    with open(input_fname, "r") as input_file:
        parse_weaponset(input_file, output_fname)

main()