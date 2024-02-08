#!/bin/sh

TARGET="pancacke"
SOURCE="src/main.c"
FLAGS="-Wall -Wextra"

set -xe
gcc $FLAGS $SOURCE -o $TARGET
