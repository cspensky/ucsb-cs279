"""
    MD5 Brute-forcing script for CS279

    Iterations required for homework assignment: 128^30 = 1.6455046e+63

    Good luck!
"""

# Native
import hashlib
import time

def gen_string(prev="", max_len=20):
    """
        Generate all possible byte patterns of max_len

    :param prev: previous string that we are appending onto the end of
    :param max_len: What is the maximum length of the string that we are testing?
    :return: A generator that will give all possible ascii strings with length between 1 and max_len
    """
    if len(prev) < max_len:
        # Only scan the ascii range
        for i in range(0, 128):
            c = chr(i)
            yield prev+c
            for x in gen_string(prev=prev+c, max_len=max_len):
                yield x

# Loop over all possible bytes and test their md5 sums.
start = time.time()
for s in gen_string(max_len=30):
    md5sum = hashlib.md5(s).hexdigest()
    if md5sum == "847dbeb849668d30722d8a67bced1c59":
        print "Found!"
        print "String: %s" % s
        print "md5: %s" % md5sum
        print "Time taken: %d" % time.time()-start
        break


