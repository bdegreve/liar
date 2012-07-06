import sys
import os
import re

srcdir = sys.argv[1]

for dirname, subdirs, fnames in os.walk(srcdir):
    for fname in fnames:
        if not fname.endswith('.cpp'):
            continue
        path = os.path.join(dirname, fname)
        lines = []
        needsWrite = False
        with open(path) as f:
            for line in f:
                if line.strip() == '#define ZLIB_WINAPI':
                    needsWrite = True
                else:
                    lines.append(line)
        if needsWrite:
            with open(path, 'w') as f:
                f.writelines(lines)
        