# Sign with our private key
gpg --armor --sign gpg_file.txt

# Encrypt using the classes public key
gpg -a -r vigna@cs.ucsb.edu -o assign1.enc --encrypt gpg_file.txt.asc 

