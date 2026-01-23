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