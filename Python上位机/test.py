from Constant import Head 

a = b'\x00\x00\x00'
a = bytearray(a)
str = "fuck"
str_bytes = str.encode(encoding="utf8")
print(str_bytes)
a.extend(str_bytes)
print(a)