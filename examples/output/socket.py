#! /usr/bin/env python

# Demonstrates the use of output.Display
#
# LiAR isn't a raytracer
# Copyright (C) 2004-2010  Bram de Greve (bramz@users.sourceforge.net)
# http://liar.bramz.net/

import sys
import subprocess
from _scene import *

(numProcesses,) = list(map(int, sys.argv[1:])) or [2]

host = output.SocketHost("127.0.0.1")
host.target = output.Display("Host", (width, height))


def start_slave(k):
    x_left = float(k) / numProcesses
    x_right = float(k + 1) / numProcesses
    cmd = [
        sys.executable,
        "socket_client.py",
        host.address,
        str(host.port),
        str(x_left),
        str(x_right),
    ]
    print(("starting child %s: %r" % (k, cmd)))
    return subprocess.Popen(cmd)


childs = [start_slave(k) for k in range(numProcesses)]
for child in childs:
    child.communicate()
