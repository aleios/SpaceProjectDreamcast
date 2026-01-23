import sys
import json
import os
import helpers
import tempfile

def write_spawn_ev(output_file, ev):
    helpers.write_str(output_file, ev.get("def", ""))

def write_music_ev(output_file, ev):
    helpers.write_str(output_file, ev.get("key", ""))
    helpers.write_float(output_file, ev.get("fade_in", 0.0))
    helpers.write_float(output_file, ev.get("fade_out", 0.0))

def write_delay_ev(output_file, ev):
    helpers.write_float(output_file, ev.get("duration", 0.0))

def write_wait_clear_ev(output_file, ev):
    helpers.write_float(output_file, ev.get("timeout", 0.0))

def write_starfield_speed_ev(output_file, ev):
    helpers.write_float(output_file, ev.get("speed", 0.0))
    helpers.write_float(output_file, ev.get("duration", 0.0))
    helpers.write_ubyte(output_file, ev.get("block", False) and 1 or 0)

def write_clear_ev(output_file, ev):
    helpers.write_ubyte(output_file, ev.get("player_projectiles", False) and 1 or 0)
    helpers.write_ubyte(output_file, ev.get("enemy_projectiles", False) and 1 or 0)
    helpers.write_ubyte(output_file, ev.get("enemies", False) and 1 or 0)
    helpers.write_ubyte(output_file, ev.get("collectables", False) and 1 or 0)

writers = [
    write_spawn_ev,
    write_music_ev,
    write_wait_clear_ev,
    write_delay_ev,
    write_starfield_speed_ev,
    write_clear_ev
]

types = [
    "spawn",
    "music",
    "wait_clear",
    "delay",
    "starfield_speed",
    "clear"
]

def parse_level(input_file, output_fname):
    data = json.load(input_file)

    initial_music = data.get("initial_music", "")
    scroll_speed = data.get("scroll_speed", 0.2)
    events = data.get("events", [])

    fd, temp_path = tempfile.mkstemp(dir=os.path.dirname(output_fname))
    try:
        with os.fdopen(fd, "wb") as output_file:
            helpers.write_magicnum(output_file, 'LDEF')

            # Write level settings
            helpers.write_str(output_file, initial_music)
            helpers.write_float(output_file, scroll_speed)

            # Write preloads.
            # preloads = data.get("preloads", {})
            # enemy_preloads = preloads.get("enemies", [])
            # projectile_preloads = preloads.get("projectiles", [])
            # helpers.write_ushort(output_file, len(enemy_preloads))
            # helpers.write_ushort(output_file, len(projectile_preloads))
            # for enemy_name in enemy_preloads:
            #     helpers.write_str(output_file, enemy_name)
            # for projectile_name in projectile_preloads:
            #     helpers.write_str(output_file, projectile_name)


            # Write events.
            helpers.write_ushort(output_file, len(events))
            for ev in events:
                pos = ev.get("pos", [0, 0])
                helpers.write_float(output_file, pos[0])
                helpers.write_float(output_file, pos[1])

                event_data = ev.get("event", {})

                ev_type_str = event_data.get("type", -1)
                ev_type = types.index(ev_type_str)
                if ev_type < 0 or ev_type >= len(writers):
                    raise Exception("[Level Convert] Error: Invalid event type")

                helpers.write_ubyte(output_file, ev_type)

                writer = writers[ev_type]
                writer(output_file, event_data)

        # Replace with true file.
        os.replace(temp_path, output_fname)
    except Exception as e:
        if os.path.exists(temp_path):
            os.remove(temp_path)
        raise e

def main():
    input_fname = sys.argv[-2]
    output_fname = sys.argv[-1]

    print("[Level Convert] ", input_fname)
    with open(input_fname, "r") as input_file:
        parse_level(input_file, output_fname)

main()