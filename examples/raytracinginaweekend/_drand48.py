class DRand48(object):
    a = 0x5DEECE66D
    c = 0xB
    m = 2 ** 48

    def __init__(self, seed=None):
        if seed is None:
            self.x = 0
        else:
            self.x = (seed << 16) % self.m | 0x330E

    def __call__(self):
        self.x = (self.a * self.x + self.c) % self.m
        return float(self.x) / self.m

    @property
    def seed48(self):
        s0 = self.x & 0xFFFF
        s1 = (self.x >> 16) & 0xFFFF
        s2 = self.x >> 32
        return s0, s1, s2

    @seed48.setter
    def seed48(self, seed):
        s0, s1, s2 = seed
        self.x = (s0 & 0xFFFF) | ((s1 & 0xFFFF) << 16) | ((s2 & 0xFFFF) << 32)
