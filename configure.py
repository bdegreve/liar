import os.path

from acoust.debug import *

dir_name = os.path.dirname(os.path.normpath(__file__))
config_name = os.path.join(dir_name, "src", "local_config.h")

def liar_have_pixeltoaster_h():
	return 1
	
def liar_have_pixeltoaster_dirtybox():
	return 1

def test(config_file, function):
	name = str(function).split()[1].upper()
	value = function()
	line = "#define %s %d" % (name, value)
	print line
	config_file.write(line + '\n')
	
def config():
	f = file(config_name, 'w')
	test(f, liar_have_pixeltoaster_h)
	test(f, liar_have_pixeltoaster_dirtybox)
	f.close()
	
def enforce_existance():
	if not os.path.exists(config_name):
		config()
		
if __name__ == "__main__":
	from optparse import OptionParser
	parser = OptionParser()
	parser.add_option("-E", "--enforce-existance", action="store_true", dest="enforce_existance", help="create src/local_config.h iff it does not exist")
	options, args = parser.parse_args()
	if len(args) > 0:
		parser.error("unexpected positional arguments")
	
	if options.enforce_existance:
		enforce_existance()
	else:
		config()
	
	