"""
    ROT encryption cracker for CS279

    NOTES:

    Alogrithm:
        encrypted[i] = rotater(orig[i], n) ^ key[i mod len(key)]

    The magic number for gzip compressed files is 1f 8b
    Also followed by 08 08 for gzip

    File seem to always end with 00 00 00

    $ xxd test.txt.gz
    0000000: 1f8b 0808 0679 1556 0003 7465 7374 2e74  .....y.V..test.t
    0000010: 7874 002b 492d 2ee1 0200 c635 b93b 0500  xt.+I-.....5.;..
    0000020: 0000                                     ..

    $ xxd test2.txt.gz
    0000000: 1f8b 0808 3a79 1556 0003 7465 7374 322e  ....:y.V..test2.
    0000010: 7478 7400 e332 b5b0 34b0 3032 36e1 1a1e  txt..2..4.026...
    0000020: 0c00 57c4 52fd d000 0000                 ..W.R.....

    Author: Chad Spensky
"""

# Native
import sys


# key_known = {0: 'i',
#                 1: 'l',
#                 2: 'i',
#                 3: 'k',
#                 # 4: 'e',   # guess
#                 6: 'e',
#                 7: 'g',
#                 8: 'a',
#                 9: 's',     # guess
#                 10: 'e',
#                 11: 'c',
#                 12: 'u',
#                 13: 'r',
#                 14: 'i',
#                 15: 't',
#                 16: 'y'     # guess
#                 }
ROT_VAL = 3 # Obtained by the nested for loops commented out below
# known values for our known plaintext attack
known_plaintext = {0: '\x1f',   # HEADER
                   1: '\x8b',   # HEADER
                   2: '\x08',   # HEADER
                   3: '\x08',   # HEADER
                   10: 's',     # orig. filename
                   11: 'e',     # orig. filename
                   12: 'c',     # orig. filename
                   13: 'r',     # orig. filename
                   14: 'e',     # orig. filename
                   15: 't',     # orig. filename
                   91: '\x00',   # checksum?
                   92: '\x00',  # checksum?
                   93: '\x00'   # checksum?
                   }


def rotate_bits(byte_val, rot_count):
    """
        rotate the byte_val to left by

        Test case:
            byte = 0b11110000
            print "%s" % bin(byte)
            print "%s" % bin(rotate_bits(byte, 2))

    :param byte_val: byte to rotate
    :param rot_count: how many times to rotate to the left
    :return:
    """

    for i in range(rot_count):
        leftmost = (0b10000000 & byte_val) >> 7
        shift_val = (0b1111111 & byte_val) << 1
        byte_val = shift_val | leftmost

    return byte_val


def test_buffer(buf):
    """
        Test if our buffer is a valid gzip file

    :param buf: Buffer that we think is a valid gzipped file
    :return: True if gzipped (with stdout dump) and False otherwise
    """

    # Import some gzip libraries
    import gzip
    from cStringIO import StringIO

    try:

        # try to open and dump the contents to stdout
        gz = gzip.GzipFile(fileobj=StringIO(buf))
        with gz as f:
            file_content = f.read()
            print "gz content: ", file_content
        return True
    except:
        return False


def get_keys(key_len, known_bytes={}, idx=0, prev_bytes=""):
    """
        Generate all possible ascii (lowercase) keys of length key_len

        ONLY GENERATES KEYS WITH LOWER-CASE ASCII CHARACTERS!

    :param key_len: length of keys to generate
    :param known_bytes: any known/static bytes in the key
    :param idx: which index are we on
    :param prev_bytes: previous bytes in recursive call
    :return:
    """

    # Known, just fill it in
    if idx in known_bytes:
        for x in get_keys(key_len,
                     known_bytes=known_bytes,
                     idx=idx + 1,
                     prev_bytes=prev_bytes + known_bytes[idx]):
            yield x

    # Otherwise, let's generate all possibilities
    elif idx < key_len:
        for val in range(ord('a'), ord('z')):   # <--- CHANGE THIS TO SEARCH OUTSIDE OF a-z
            val = chr(val)
            for x in get_keys(key_len,
                     known_bytes=known_bytes,
                     idx=idx + 1,
                     prev_bytes=prev_bytes + val):
                yield x
    else:
        # termination cause, just return the generated key
        yield prev_bytes


def decrypt(cryptogram, decryption_key, rot_val):
    """
        Given our key, and number of bits rotated, decrypt the payload

    :param cryptogram: encrypted content
    :param decryption_key: decryption key
    :param rot_val: The number of rotations for this decryption
    :return: decrypted value of cryptogram with key applied in blocks
    """

    i = 0
    output = ""

    # Loop over all of the bytes in the encrypted string in chunks of the decryption key
    while i < len(cryptogram):
        # Algorithm as described in the homework
        key_idx = i % len(decryption_key)
        output += chr(rotate_bits(ord(cryptogram[i]) ^ ord(decryption_key[key_idx]), rot_val))
        i += 1

    return output


def crack_key(known_plaintext, cryptogram, keylen=17, idx=0, key_uncovered={}):
    """
        Crack the key using a known plaintext attack

    :param known_plaintext: Dictionary of known plaintext {file offset -> value}
    :param cryptogram: entire cryptogram that we are trying to decrypt
    :param keylen: length of key that we are cracking
    :param idx: current index in the file
    :param key_uncovered: dictionary to keep track of key values that have been uncovered
    :return:
    """
    if idx == len(cryptogram):
        return key_uncovered

    if idx in known_plaintext:

        # Loop over all possible values... Ideally we'll hit sooner than later.
        for i in range(0, 256):
            decrypted_val = decrypt(cryptogram[idx], chr(i), ROT_VAL)
            if decrypted_val == known_plaintext[idx]:
                key_uncovered[idx % keylen] = chr(i)
                return crack_key(known_plaintext, cryptogram, idx=idx+1, key_uncovered=key_uncovered)

        # if we got here and didn't find anything, we have big issues!
        print "ERROR: Known plaintext attack failed on index %d" % idx
        return crack_key(known_plaintext, cryptogram, idx=idx+1, key_uncovered=key_uncovered)
    else:
        return crack_key(known_plaintext, cryptogram, idx=idx+1, key_uncovered=key_uncovered)


# # Really rough (non-general) solution to uncover some known plaintext, and find the rot value
# for rot_val in range(0, 8):
#     for key_val1 in range(0x61, 0x7b):
#         for key_val2 in range(0x61, 0x7b):
#             for key_val3 in range(0x61, 0x7b):
#                 for key_val4 in range(0x61, 0x7b):
#                     b1 = rotate_bits(ord(contents_enc[0]) ^ key_val1, rot_val)
#                     b2 = rotate_bits(ord(contents_enc[1]) ^ key_val2, rot_val)
#                     b3 = rotate_bits(ord(contents_enc[2]) ^ key_val3, rot_val)
#                     b4 = rotate_bits(ord(contents_enc[3]) ^ key_val4, rot_val)
#
#                     if b1 == 0x1f and b2 == 0x8b and b3 == 0x08 and b4 == 0x08:
#                         print "cracked!  rot: %d, key: %s %s %s %s" % (rot_val, chr(key_val1), chr(key_val2),
#                                                                     chr(key_val3), chr(key_val4))

# Read in our encrypted file
file_in = open("secretfile.txt.gz.enc", "r")
contents_enc = file_in.read()
file_in.close()

key_known = crack_key(known_plaintext, contents_enc)

print "* Uncovered the following key values using a known-plain-text attack:"
for i in range(17):
    if i in key_known:
        print "  %d: %s" % (i,`key_known[i]`)
    else:
        print "  %d: ??" % i

print ""
bytes_left = (17-len(key_known.keys()))
print "* Trying to crack the %d remaining bytes..."% bytes_left

total_keys = bytes_left**26
print "* Attempting %d different key combinations" % total_keys

idx = 0
prct = 0
for k in get_keys(17, known_bytes=key_known):
    dec = decrypt(contents_enc, k, ROT_VAL)
    if test_buffer(dec):
        print "Cracked it!"
        print "The key was: %s" % k
        f = open("secrefile.txt.gz", "w+")
        f.write(dec)
        f.close()
        print "file saved to: secrefile.txt.gz"
        sys.exit(0)

    idx += 1
    if idx % 1000 == 0:
        print "%d keys attempted..." % idx
