import sys
import os
import re

srcdir = sys.argv[1]

for dirname, subdirs, fnames in os.walk(srcdir):
    for fname in fnames:
        if not fname.endswith('.cpp'):
            continue
        path = os.path.join(dirname, fname)
        lines = ['#include <algorithm>\n'] # apparently this is necessary/
        with open(path) as f:
            for line in f:
                if line.strip() == '#define ZLIB_WINAPI':
                    continue                    
                lines.append(line)
        with open(path, 'w') as f:
            f.writelines(lines)
        