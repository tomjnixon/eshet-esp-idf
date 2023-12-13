#!/bin/bash
find src/ include/ -name '*.cpp' -or -name '*.hpp' | xargs clang-format -i
