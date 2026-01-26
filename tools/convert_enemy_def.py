import sys
import json
import struct
import helpers
import math

# Define the opcode mapping clearly
EVENT_CONFIG = {
    "moveto":      0,
    "stopmoving":  1,
    "startfiring": 2,
    "stopfiring":  3,
    "delay":       4,
    "destroy":     5,
    "exitscreen":  6,
    "repeat":      7
}

def target_to_byte(target):
    t = target.lower()
    if t == "point": return 0
    if t == "player_initial": return 1
    if t == "player_target": return 2
    if t == "direction": return 3
    if t == "sine": return 4
    return 0

def write_events(output_file, events):
    helpers.write_ubyte(output_file, len(events))
    for event in events:
        ev_type = event.get('type', '').lower()
        opcode = EVENT_CONFIG.get(ev_type)

        if opcode is None:
            print(f"[Enemy Def] Invalid or unknown event type: {ev_type}")
            continue

        # Write opcode
        helpers.write_ubyte(output_file, opcode)

        if ev_type == "moveto":
            target = event.get('target', '')
            helpers.write_ubyte(output_file, target_to_byte(target))
            helpers.write_float(output_file, event.get('speed', 0.0))
            if target.lower() == "point":
                helpers.write_float(output_file, event.get('x', 0.0))
                helpers.write_float(output_file, event.get('y', 0.0))
            elif target.lower() == "direction":
                helpers.write_float(output_file, math.radians(event.get('angle', 0.0)))
                helpers.write_float(output_file, math.radians(event.get('angle_step', 0.0)))
                helpers.write_float(output_file, event.get('duration', 0.0))
            elif target.lower() == "sine":
                helpers.write_float(output_file, math.radians(event.get('angle', 0.0)))
                scale = max(0.0, min(event.get('period', 0.0), 1.0))
                omega = 2.0 * math.pi * (scale * 2.0)
                helpers.write_float(output_file, omega)
                helpers.write_float(output_file, event.get('amplitude', 0.0))
                helpers.write_float(output_file, event.get('duration', 0.0))
        elif ev_type == "startfiring":
            helpers.write_emitter(output_file, event)
            # helpers.write_str(output_file, event.get('projectile', ''))
            # helpers.write_float(output_file, event.get('delay', 0.0))
            # helpers.write_float(output_file, math.radians(event.get('start_angle', 0.0)))
            # helpers.write_float(output_file, math.radians(event.get('step_angle', 0.0)))
            # helpers.write_ubyte(output_file, event.get('spawns_per_step', 1))
        elif ev_type == "delay":
            helpers.write_float(output_file, event.get('duration', 0.0))
        elif ev_type == "exitscreen":
            helpers.write_float(output_file, event.get('speed', 0.5))
        elif ev_type == "repeat":
            helpers.write_ushort(output_file, event.get('count', 0))
            helpers.write_ushort(output_file, event.get('target', 0))
        

def parse_proj_def(input_file, output_fname):
    data = json.load(input_file)

    texture = data.get('texture', '')
    animation = data.get('animation', '')
    idle_key = data.get('idle_key', '')
    left_key = data.get('left_key', '')
    right_key = data.get('right_key', '')

    health = data.get('health', 1)

    events = data.get('events', [])

    with open(output_fname, "wb") as output_file:
        helpers.write_magicnum(output_file, 'EDEF')
        helpers.write_str(output_file, texture)
        helpers.write_str(output_file, animation)
        helpers.write_str(output_file, idle_key)
        helpers.write_str(output_file, left_key)
        helpers.write_str(output_file, right_key)

        helpers.write_ushort(output_file, health)

        write_events(output_file, events)


def main():
    input_fname = sys.argv[-2]
    output_fname = sys.argv[-1]

    print("[Enemy Def] ", input_fname)
    with open(input_fname, "r") as input_file:
        parse_proj_def(input_file, output_fname)

main()