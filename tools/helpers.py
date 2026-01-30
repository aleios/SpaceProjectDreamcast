import struct
import math

def write_str(output_file, val):
    output_file.write(struct.pack('<H', len(val)))
    output_file.write(val.encode())

def write_float(output_file, val):
    output_file.write(struct.pack("<f", val))

def write_ubyte(output_file, val):
    output_file.write(struct.pack("B", val))

def write_ushort(output_file, val):
    output_file.write(struct.pack("<H", val))

def write_magicnum(output_file, magicnum):
    output_file.write(magicnum.encode())

def write_emitter(output_file, emitter):
    write_str(output_file, emitter.get("projectile", ""))
    write_ushort(output_file, emitter.get("spawns_per_step", 1))
    write_float(output_file, emitter.get("delay", 1000.0))
    write_float(output_file, math.radians(emitter.get("start_angle", 0.0)))
    write_float(output_file, math.radians(emitter.get("step_angle", 0.0)))
    write_float(output_file, emitter.get("speed", 0.5))
    write_float(output_file, emitter.get("lifetime", 1000.0))

    write_float(output_file, emitter.get("offset", [0.0, 0.0])[0])
    write_float(output_file, emitter.get("offset", [0.0, 0.0])[1])

    # Target and tracking
    target = emitter.get("target", 0)
    write_ubyte(output_file, target)

    # If we have a tracking target
    if target > 0:
        write_ubyte(output_file, emitter.get("target_tracking", 0))
        write_float(output_file, float(emitter.get("tracking_delay", 0.0)))

def write_weapons(output_file, weapons):
    write_ushort(output_file, len(weapons))
    for weap in weapons:
        emitters = weap.get("emitters", [])
        write_ushort(output_file, len(emitters))
        for emitter in emitters:
            write_emitter(output_file, emitter)
