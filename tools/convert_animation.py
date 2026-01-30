#
# Converts JSON animation files into binary
#

import json
import sys
import struct
import helpers

global_origin = None

def write_header(output_file, texture, num_animations):
    helpers.write_magicnum(output_file, 'ASAM')
    helpers.write_str(output_file, texture)
    helpers.write_ushort(output_file, num_animations)

def write_animation(output_file, animation):
    num_frames = len(animation['frames'])

    # Write name and length of it.
    helpers.write_str(output_file, animation['name'])

    helpers.write_float(output_file, animation['fps'])
    helpers.write_ubyte(output_file, animation['loop_mode'])

    helpers.write_float(output_file, animation['origin'][0])
    helpers.write_float(output_file, animation['origin'][1])

    helpers.write_ushort(output_file, num_frames)
    for frame in animation['frames']:
        helpers.write_float(output_file, frame[0])
        helpers.write_float(output_file, frame[1])
        helpers.write_float(output_file, frame[2])
        helpers.write_float(output_file, frame[3])
        helpers.write_float(output_file, frame[4]) # Width
        helpers.write_float(output_file, frame[5]) # Height


def parse_animation(key, anim_data, atlas_width, atlas_height):
    data_frames = anim_data['frames']
    if not data_frames or type(data_frames) != list:
        print(f"Error: Empty or invalid 'frames' for animation '{key}'")
        exit(1)

    frames = data_frames

    for i, frame in enumerate(frames):
        if len(frame) != 4:
            print(f"Error: frames must contain 4 components. animation '{key}' index '{i}'")
            exit(1)
        

    flip_h: bool = anim_data.get("flip_h", False)
    flip_v: bool = anim_data.get("flip_v", False)
    fps: float = anim_data.get("fps", 0.0)
    loop_mode: int = anim_data.get("loop_mode", 0)

    origin = anim_data.get("origin", None)

    if not origin and not global_origin:
        origin = [ 0.0, 0.0 ]
    elif not origin and global_origin:
        origin = global_origin

    if type(flip_h) != bool:
        print(f"flip_h contains invalid type for animation '{key}'")
        exit(1)
    if type(flip_v) != bool:
        print(f"flip_v contains invalid type for animation '{key}'")
        exit(1)

    if type(fps) != float:
        print(f"fps must be a valid number for animation '{key}'")
        exit(1)

    if type(loop_mode) != int:
        print(f"loop_mode must be a valid identifier '{key}'")
        exit(1)

    if loop_mode < 0 or loop_mode > 2:
        print(f"Invalid or unknown loop_mode. Must be between 0 and 2. (0 = forward, 1 = backward, 2 = ping pong) '{key}'")
        exit(1)

    # Compute UVs
    for i, frame in enumerate(frames):

        # Append Width and Height in pixels
        frame.append(frame[2])
        frame.append(frame[3])

        # Calculate width and height of frame.
        frame[2] = frame[0] + frame[2]
        frame[3] = frame[1] + frame[3]

        # Normalize UVs to [0, 1]
        frame[0] /= atlas_width
        frame[1] /= atlas_height
        frame[2] /= atlas_width
        frame[3] /= atlas_height

        # Flip UVs if applicable.
        if flip_h:
            tmp = frame[0]
            frame[0] = frame[2]
            frame[2] = tmp

        if flip_v:
            tmp = frame[1]
            frame[1] = frame[3]
            frame[3] = tmp

    # Convert fps to milliseconds
    if fps > 0.0:
        fps = (1000.0 / fps)

    return {
        "name": key,
        "frames": frames,
        "fps": fps,
        "loop_mode": loop_mode,
        "origin": origin
    }

def parse_animation_set(input_file, output_fname):
    # Read input as JSON
    data = json.load(input_file)

    # Find metadata
    metadata = data['_meta']

    if not metadata:
        print("Error: No metadata block")
        exit(1)

    texture = metadata.get("texture", None)
    if not texture:
        print("Error: Missing texture name")
        exit(1)

    atlas_width = metadata.get("atlas_width", 0)
    atlas_height = metadata.get("atlas_height", 0)

    global global_origin
    global_origin = metadata.get("origin", None)

    if atlas_width <= 0 or atlas_height <= 0:
        print("Error: Missing or invalid 'altas_width' or 'atlas_height'")
        exit(1)

    # Parse keys as animations
    animations = []
    for key in data:
        if key == '_meta':
            continue
        anim = parse_animation(key, data[key], atlas_width, atlas_height)
        animations.append(anim)

    num_animations = len(animations)

    with open(output_fname, "wb") as output_file:
        write_header(output_file, texture, num_animations)
        for animation in animations:
            write_animation(output_file, animation)

def main():
    input_fname = sys.argv[-2]
    output_fname = sys.argv[-1]

    print("[Animation Convert] ", input_fname)
    with open(input_fname, "r") as input_file:
        parse_animation_set(input_file, output_fname)

main()