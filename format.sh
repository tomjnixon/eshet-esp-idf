#!/bin/bash
git ls-files -co --exclude-standard | grep '\.[hc]pp$' | xargs clang-format -i
