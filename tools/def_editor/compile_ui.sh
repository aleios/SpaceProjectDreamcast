#!/bin/bash

echo "Running UIC"

for file in ui/*.ui; do
    [[ -e "$file" ]] || continue

    dir_path=$(dirname "$file")
    full_filename=$(basename "$file")
    
    raw_name="${full_filename%.ui}"
    pascal_name=$(echo "$raw_name" | sed -E 's/[_-]+/ /g' | sed -E 's/\b([a-z])/\U\1/g' | sed 's/ //g')
    output_file="${dir_path}/${pascal_name}.py"

    echo "Converting: $file -> $output_file"

    # Run pyuic6
    pyuic6 "$file" -o "$output_file"
done