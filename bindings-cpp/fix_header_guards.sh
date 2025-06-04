#!/bin/bash

# Fix header guards in library headers

set -e

echo "Fixing header guards..."

# Fix header guards in library headers
find /home/ludvi/stdbfork/SpacetimeDB/bindings-cpp/library/include -name "*.h" | while read -r file; do
    if grep -q "SPACETIMEDB_SDK_" "$file"; then
        echo "Fixing header guards in: $file"
        sed -i 's/SPACETIMEDB_SDK_/SPACETIMEDB_LIBRARY_/g' "$file"
    fi
done

# Also check for any remaining sdk references in headers
find /home/ludvi/stdbfork/SpacetimeDB/bindings-cpp -name "*.h" -o -name "*.cpp" | while read -r file; do
    if grep -q "spacetimedb_sdk_" "$file"; then
        echo "Fixing remaining sdk references in: $file"
        sed -i 's/spacetimedb_sdk_/spacetimedb_library_/g' "$file"
    fi
done

echo "Header guard fixes complete!"