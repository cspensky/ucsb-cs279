#!/usr/bin/env python
"""
    Chad Spensky, cspensky@cs.ucsb.edu

    This program will take a list of hostnames (one per line in a file) and send
    an NX record when a DNS request goes out for any of those names.
"""

# Native
import argparse
import sys
import logging
logger = logging.getLogger(__name__)
logging.basicConfig()

# 3rd party
from scapy.all import *

# Global for our block list
block_list = []


def dns_block(pkt):
    """
        Callback for Scapy

        Will sniff DNS traffic and send a NXDOMAIN for any query that exists
        in our list.

        NOTE: Assumes that block_list is a global!

    :param pkt: Scapy packet
    :return:
    """
    global block_list

    # Is it in our block list?
    print pkt[DNSQR].qname
    print block_list
    if pkt[UDP].dport != 53 or pkt[DNSQR].qname not in block_list:
        return
    print "Sent!"

    logger.info("Saw a dns request for %s from %s.  Sending NX response!" % (
        pkt[IP].src, pkt[DNSQR].qname))

    # Construct our packet
    spoofed_pkt = Ether(dst=pkt[Ether].src, src=pkt[Ether].dst,
                        type=pkt[Ether].type) / \
                    IP(dst=pkt[IP].src, src=pkt[IP].dst) / \
                    UDP(dport=pkt[UDP].sport, sport=pkt[UDP].dport) / \
                    DNS(id=pkt[DNS].id, qd=pkt[DNS].qd, aa=0, qr=1, opcode=0,
                        tc=0, rd=1, ra=1, z=0, rcode=3, qdcount=1, ancount=0,
                        nscount=0, arcount=0, an=None, ns=None, ar=None)

    # Send it on the wire
    sendp(spoofed_pkt)

if __name__ == "__main__":

    # Get some user input
    parser = argparse.ArgumentParser()
    parser.add_argument("--list", "-l", default=None, type=str,
                        help="Filename containing a list of host names to "
                             "block.")
    parser.add_argument("--iface", "-i", default="eth0", type=str,
                        help="Interface to listen/send packets on.")

    args = parser.parse_args()

    # Make sure they provided a valid filename
    if args.list is None:
        logger.error("No filename given.")
        parser.print_help()
        sys.exit(0)
    elif not os.path.exists(args.list):
        logger.error("The file (%s) does not exist."%args.list)
        parser.print_help()
        sys.exit(0)

    # Parse our list of domains
    try:
        f = open(args.list, "r")
        for hostname in f:
            tmp = hostname.strip()
            if tmp.endswith("."):
                block_list.append(tmp)
            else:
                block_list.append(tmp+".")
        f.close()
    except:
        logger.error("Could not parse given file.")
        sys.exit(0)

    try:
        sniff(iface=args.iface, prn=dns_block, filter="udp and port 53")
    except:
        logger.error("Could not open interface for sniffing.")
        logger.error("Are you root?  Also, scapy is flaky, you may just "
                     "need to try again...")
        sys.exit(0)
