import os
import sys
import subprocess
from optparse import OptionParser

def prepend_dirs(varname, dirs):
	dirs = dirs + os.getenv(varname, "").split(os.pathsep)
	os.environ[varname] = os.pathsep.join(dirs)

parser = OptionParser()
parser.disable_interspersed_args()
parser.add_option("-l", "--with-library", action="append", metavar="<path>", help="add dirname of <path> to library search path")
options, args = parser.parse_args()

lib_dirs = [os.path.dirname(path) for path in options.with_library]

if "win" in sys.platform:
	prepend_dirs("PATH", lib_dirs)

open("foo.txt", "w").write(repr(args))
subprocess.check_call(args) 