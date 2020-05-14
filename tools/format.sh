#!/bin/sh
echo $PWD
find $PWD -type f \( -name '*.h' -o -name '*.cc' \) -exec echo "Formatting {}" \; -exec clang-format -style=file -i {} \;
