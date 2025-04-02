import asyncore
import socket
import struct

class BufferUnderflow(Exception):
    pass

class EchoHandler(asyncore.dispatcher):
    def __init__(self, sock, renderengine):
        self.__recvsize = 4096
        self.__readbuffer = bytearray()
        self.__writebuffer = bytearray()
        asyncore.dispatcher.__init__(self, sock=sock)
        self.__renderengine = renderengine
        self.__result = None

    def handle_read(self):
        received = self.recv(self.__recvsize)
        if not received:
            self.close()
        self.__readbuffer += received
        while True:
            try:
                buffer = self.__readbuffer
                self.__process_buffer()
            except BufferUnderflow:
                self.__readbuffer = buffer
                return

    def handle_write(self):
        buffer = self.__writebuffer
        n = self.send(buffer)
        self.__writebuffer = buffer[n:]

    def handle_close(self):
        if self.__result:
            self.__renderengine.end_result(self.__result)

    def __process_buffer(self):
        engine = self.__renderengine
        code, = self.__readstruct("<h")
        print ("code={}".format(code))
        if code == 0: # scSample
            x, y, r, g, b, z, a, w = self.__readstruct("<dddddddd")
            i, j = int(x * engine.size_x), int(y * engine.size_y)
            address = j * engine.size_x + i
            p = self.__displayBuffer[address]
            p[0] += 1.0
            p[1] += g
            p[2] += b
            p[3] += a
            self.__weightBuffer[address] += w
            self.__result.layers[0].rect = [[x * w for x in p] for p, w in zip(self.__displayBuffer, self.__weightBuffer)]
        elif code == 4: # scIsCanceling
            self.send(struct.pack("<B", 0))
        elif code == 1: # scBeginRender
            pixel_count = engine.size_x * engine.size_y
            self.__result = engine.begin_result(0, 0, engine.size_x, engine.size_y)
            self.__displayBuffer = [[0.0, 0.0, 0.0, 0.0] for k in range(pixel_count)]
            self.__weightBuffer = [0] * pixel_count
        elif code == 2: # scEndRender
            self.close()
        elif code == 3: # scResolution
            self.send(struct.pack("<II", engine.size_x, engine.size_y))
        else:
            print("OOPS, unexpected code {}".format(code))
            self.close()


    def __readstruct(self, fmt):
        n = struct.calcsize(fmt)
        buf = self.__readn(n)
        assert len(buf) == n
        return struct.unpack(fmt, buf)

    def __readstring(self):
        n, = self.__readstruct("<Q")
        return self.__readn(n)

    def __readn(self, n):
        buffer = self.__readbuffer
        while len(buffer) < n:
            raise BufferUnderflow()
        result, self.__readbuffer = buffer[:n], buffer[n:]
        return result


class ServerThing(asyncore.dispatcher):
    def __init__(self, renderengine):
        asyncore.dispatcher.__init__(self)
        self.renderengine = renderengine
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.set_reuse_addr()
        self.bind(('localhost', 0))
        self.address = self.socket.getsockname()
        self.listen(1)

    def handle_accepted(self, sock, addr):
        print('Incoming connection from %s' % repr(addr))
        EchoHandler(sock, self.renderengine)
        self.handle_close()

    def handle_close(self):
        self.close()


def render_scene(self, scene):
    from render_liar import export
    import bpy
    import subprocess

    addon_prefs = bpy.context.user_preferences.addons[__package__].preferences
    python_binary = addon_prefs.python_binary
    if not _test_liar(python_binary):
        self.report({'ERROR'}, "{!r} is not a valid path to a Python binary with the LiAR package installed. Adjust the path in User Preferences, and try again.".format(python_binary))
        return False

    path = export.export_scene(scene)
    port = 8215

    server = ServerThing(self)
    process = subprocess.Popen([python_binary, path, "--remote", "%s:%d" % server.address])
    asyncore.loop()
    process.wait()
    return True


def _test_liar(python_binary):
    import subprocess
    try:
        output = subprocess.check_output([python_binary, "-c", "import liar"])
    except (subprocess.CalledProcessError, OSError):
        return False
    else:
        return True
