#!/bin/sh

xxd -i < $1 | sed 's/\([0-9a-f]\)$/\0, 0x00/' > $2
