import struct
fin = open("output", "rb")
for i in range(10):
    print(struct.unpack('f', fin.read(4)))
for i in range(10):
    print(struct.unpack('c', fin.read(1)), end="")