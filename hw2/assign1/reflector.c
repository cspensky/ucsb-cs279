/* Chad Spensky, cspensky@cs.ucsb.edu
 *
 *
 * Ref (Argp): https://www.gnu.org/software/libc/manual/html_node/Argp-Example-3.html#Argp-Example-3

 *
 * */

#include <stdlib.h>
#include <argp.h>
#include <arpa/inet.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <ctype.h>
#include <string.h> // memcpy
#include <net/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
//#include <linux/if_arp.h>
#include <sys/ioctl.h>

// Local
//#include "reflector.h"
// External libraries
#include <pcap.h>

/* Ethernet addresses are 6 bytes */
#define ETHER_ADDR_LEN	6

/* Ethernet header */
struct sniff_ethernet {
	u_char ether_dhost[ETHER_ADDR_LEN]; /* Destination host address */
	u_char ether_shost[ETHER_ADDR_LEN]; /* Source host address */
	u_short ether_type; /* IP? ARP? RARP? etc */
};

/* IP header */
struct sniff_ip {
	u_char ip_vhl;		/* version << 4 | header length >> 2 */
	u_char ip_tos;		/* type of service */
	u_short ip_len;		/* total length */
	u_short ip_id;		/* identification */
	u_short ip_off;		/* fragment offset field */
#define IP_RF 0x8000		/* reserved fragment flag */
#define IP_DF 0x4000		/* dont fragment flag */
#define IP_MF 0x2000		/* more fragments flag */
#define IP_OFFMASK 0x1fff	/* mask for fragmenting bits */
	u_char ip_ttl;		/* time to live */
	u_char ip_p;		/* protocol */
	u_short ip_sum;		/* checksum */
	struct in_addr ip_src,ip_dst; /* source and dest address */
};
#define IP_HL(ip)		(((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)		(((ip)->ip_vhl) >> 4)

/* TCP header */
typedef u_int tcp_seq;

struct sniff_tcp {
	u_short th_sport;	/* source port */
	u_short th_dport;	/* destination port */
	tcp_seq th_seq;		/* sequence number */
	tcp_seq th_ack;		/* acknowledgement number */
	u_char th_offx2;	/* data offset, rsvd */
#define TH_OFF(th)	(((th)->th_offx2 & 0xf0) >> 4)
	u_char th_flags;
#define TH_FIN 0x01
#define TH_SYN 0x02
#define TH_RST 0x04
#define TH_PUSH 0x08
#define TH_ACK 0x10
#define TH_URG 0x20
#define TH_ECE 0x40
#define TH_CWR 0x80
#define TH_FLAGS (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
	u_short th_win;		/* window */
	u_short th_sum;		/* checksum */
	u_short th_urp;		/* urgent pointer */
};

typedef struct arphdr {
    u_int16_t htype;    /* Hardware Type           */
    u_int16_t ptype;    /* Protocol Type           */
    u_char hlen;        /* Hardware Address Length */
    u_char plen;        /* Protocol Address Length */
    u_int16_t oper;     /* Operation Code          */
    u_char sha[6];      /* Sender hardware address */
    u_char spa[4];      /* Sender IP address       */
    u_char tha[6];      /* Target hardware address */
    u_char tpa[4];      /* Target IP address       */
}arphdr_t;


/* ethernet headers are always exactly 14 bytes */
#define SIZE_ETHERNET 14
#define ETH_ARP 0x0806
#define ETH_IP 0x0800
#define ARP_REQUEST 0x001   /* ARP Request             */
#define ARP_REPLY 0x002     /* ARP Reply               */



// Functions
int isValidMAC(const char* mac);
int isValidIP(char *ipAddress);

const char *argp_program_version =
  "reflector 1.0a";
const char *argp_program_bug_address =
  "<cspensky@cs.ucsb.edu>";

/* Program documentation. */
static char doc[] =
  "IP Reflector -- This application will impersonate a victim and relayer.  All traffic directed at the victim will be sent back to the source from the relayer.";

/* A description of the arguments we accept. */
static char args_doc[] = "<iface>";

/* The options we understand. */
static struct argp_option options[] = {
  {"verbose", 		 	'v', 0,     OPTION_ARG_OPTIONAL, 	"Produce verbose output" },
  {"victim-ip",    		'i', "IP",     	0,  "Victim IP Address" },
  {"victim-ethernet",   'e', "MAC",     0, 	"Victim Ethernet Address" },
  {"relayer-ip",   		'p', "IP", 		0,	"Relayer IP Address" },
  {"relayer-ethernet",	't', "MAC", 	0,	"Relayer Ethernet Address" },
  { 0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments
{
  int verbose;
  char *victim_ip;
  char *victim_mac;
  char *relayer_ip;
  char *relayer_mac;
  char *iface;
};

/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;

  switch (key)
  {

    case 'v':
      arguments->verbose = 1;
      break;
    case 'i':
      arguments->victim_ip = arg;
      if (!isValidIP(arguments->victim_ip)) {
    	  fprintf(stderr, "Victim IP is not valid.");
    	  argp_usage(state);
      }
      break;
    case 'e':
          arguments->victim_mac = arg;
          if (!isValidMAC(arguments->victim_mac)) {
        	  fprintf(stderr, "Victim MAC is not valid.");
        	  argp_usage(state);
          }
          break;
    case 'p':
          arguments->relayer_ip = arg;
          if (!isValidIP(arguments->relayer_ip)) {
        	  fprintf(stderr, "Relayer IP is not valid.");
        	  argp_usage(state);
          }
          break;
    case 't':
          arguments->relayer_mac = arg;
          if (!isValidMAC(arguments->relayer_mac)) {
        	  fprintf(stderr, "Relayer MAC is not valid.");
        	  argp_usage(state);
          }
          break;

    case ARGP_KEY_ARG:
    	if (state->arg_num == 0) {
    		arguments->iface = arg;
    	}
      /* Just ignore arguments */
      break;
    case ARGP_KEY_END:
      /* Just ignore arguments */
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

/**
 * Function to verify IP addresses
 *
 * Ref: https://stackoverflow.com/questions/791982/determine-if-a-string-is-a-valid-ip-address-in-c
 */

int isValidIP(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

/**
 * Function to verify our MAC addresses
 *
 * Ref: https://stackoverflow.com/questions/4792035/how-do-you-validate-that-a-string-is-a-valid-mac-address-in-c
 */
int isValidMAC(const char* mac) {
    int i = 0;
    int s = 0;

    while (*mac) {
       if (isxdigit(*mac)) {
          i++;
       }
       else if (*mac == ':') {

          if (i == 0 || i / 2 - 1 != s)
            break;

          ++s;
       }
       else {
           s = -1;
       }


       ++mac;
    }

    return (i == 12 && (s == 5 || s == 0));
}

/*
 * IPs expected in network byte order
 */
int reflect_packets(in_addr_t victim_ip, unsigned char victim_mac[], in_addr_t relayer_ip, unsigned char relayer_mac[], char *dev) {
	// Libpcap stuff
	pcap_t *handle;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct bpf_program fp;		/* The compiled filter expression */
	char filter_exp[] = "ip or arp";	/* The filter expression */
	const u_char *packet;		/* The actual packet */
	struct pcap_pkthdr header;	/* The header that pcap gives us */

	// Headers
	const struct sniff_ethernet *ethernet; /* The ethernet header */
	const struct sniff_ip *ip; /* The IP header */
	struct arphdr *arpheader;       /* Pointer to the ARP header              */
	u_int size_ip;

	// Open our sniffing device
	handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
		return(2);
	}
	// Support ethernet?
	if (pcap_datalink(handle) != DLT_EN10MB) {
		fprintf(stderr, "Device %s doesn't provide Ethernet headers - not supported\n", dev);
		return(2);
	}
	// Compile our filter to only get ARP and IP
	if (pcap_compile(handle, &fp, filter_exp, 0, 0) == -1) {
		fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
		return(2);
	}
	if (pcap_setfilter(handle, &fp) == -1) {
		fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
		return(2);
	}

	char ip_src_str[INET_ADDRSTRLEN];
	char ip_dest_str[INET_ADDRSTRLEN];

	// Keep track of our attacker
	in_addr_t attacker_ip = 0;
	unsigned char attacker_mac[6] = {0,0,0,0,0,0};
	
	while (1) {
		/* Grab a packet */
		packet = pcap_next(handle, &header);

		// Setup our header pointer
		ethernet = (struct sniff_ethernet*)(packet);

		// Ignore bogus reads
		if (header.len == 0 || packet == NULL) {
			continue;
		}


		if (htons(ethernet->ether_type) == ETH_ARP) {
			// Get our arp header and extract useful info

			arpheader = (struct arphdr *)(packet+SIZE_ETHERNET);
			inet_ntop(AF_INET, &(arpheader->spa), ip_src_str, INET_ADDRSTRLEN);
			inet_ntop(AF_INET, &(arpheader->tpa), ip_dest_str, INET_ADDRSTRLEN);

			in_addr_t destip;
			memcpy((void *)&destip, (void *)arpheader->tpa, sizeof(arpheader->tpa));

			// Reply to all ARP packets for both IPs to make us seem alive!
			if (destip == victim_ip) {
				printf("* Got an ARP for our target IP from %s. Replying... ", ip_src_str);

				// Make the packet a reply
				arpheader->oper = htons(ARP_REPLY);
				// Flip the source to destination
				memcpy((void *)arpheader->tpa,(void *)arpheader->spa,sizeof(arpheader->tpa));
				memcpy((void *)arpheader->tha,(void *)arpheader->sha,sizeof(arpheader->tha));
				memcpy((void *)ethernet->ether_dhost, (void *)ethernet->ether_shost, sizeof(ethernet->ether_dhost));


				// Fill in our address
				memcpy((void *)arpheader->sha, (void *)victim_mac, sizeof(arpheader->sha));
				memcpy((void *)arpheader->spa, (void *)&victim_ip, sizeof(arpheader->spa));
				memcpy((void *)ethernet->ether_shost, (void *)victim_mac, sizeof(ethernet->ether_shost));

				// Send our response
				int sent = 0;
				if ((sent = pcap_inject(handle,packet,header.len)) == -1) {
					pcap_perror(handle,0);
				}
				printf("(Sent %d bytes)\n", sent);

			} else if (ntohl(*arpheader->tpa) == relayer_ip) {
				printf("* Got an ARP for our relayer IP from %s. Replying... ", ip_src_str);

				// Make the packet a reply
				arpheader->oper = htons(ARP_REPLY);
				// Flip the source to destination
				memcpy((void *)arpheader->tpa, (void *)arpheader->spa,sizeof(arpheader->tpa));
				memcpy((void *)arpheader->tha, (void *)arpheader->sha,sizeof(arpheader->tha));
				memcpy((void *)ethernet->ether_dhost, (void *)ethernet->ether_shost, sizeof(ethernet->ether_dhost));


				// Fill in our address
				memcpy((void *)arpheader->sha, (void *)relayer_mac, sizeof(arpheader->sha));
				memcpy((void *)arpheader->spa, (void *)&relayer_ip, sizeof(arpheader->spa));
				memcpy((void *)ethernet->ether_shost, (void *)relayer_mac, sizeof(ethernet->ether_shost));

				// Send our response
				int sent = 0;
				if ((sent = pcap_inject(handle,packet,header.len)) == -1) {
					pcap_perror(handle,0);
				}
				printf("(Sent %d bytes)\n", sent);
			}


		} else if (htons(ethernet->ether_type) == ETH_IP) {
			ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
			size_ip = IP_HL(ip)*4;
			if (size_ip < 20) {
				printf("   * Invalid IP header length: %u bytes\n", size_ip);
				continue;
			}

			// Extract our target ip
			in_addr_t dest_ip = ip->ip_dst.s_addr;

			// now get it back and print it
			inet_ntop(AF_INET, &(ip->ip_src.s_addr), ip_src_str, INET_ADDRSTRLEN);
			inet_ntop(AF_INET, &(ip->ip_dst.s_addr), ip_dest_str, INET_ADDRSTRLEN);

			// Is this packet directed at our victim?
			if (dest_ip == victim_ip) {
				printf("Got an IP packet directed to our victim from %s. Relaying back... ",ip_dest_str);

				// Update our attacker info
				memcpy((void *)attacker_mac, (void *)ethernet->ether_shost, sizeof(attacker_mac));
				memcpy((void *)&attacker_ip, (void *)&(ip->ip_src.s_addr), sizeof(attacker_ip));

				// Update the fields for replaying this packet
				//
				//	WARNING: We may have to update the checksum!
				memcpy((void *)ethernet->ether_shost, (void *)relayer_mac, sizeof(ethernet->ether_shost));
				memcpy((void *)ethernet->ether_dhost, (void *)attacker_mac, sizeof(ethernet->ether_dhost));

				memcpy((void *)&(ip->ip_src.s_addr), (void *)&relayer_ip, sizeof(ip->ip_src.s_addr));
				memcpy((void *)&(ip->ip_dst.s_addr), (void *)&attacker_ip, sizeof(ip->ip_dst.s_addr));


				// replay this packet to it's source from our relayer
				int sent = 0;
				if ((sent = pcap_inject(handle,packet,header.len)) == -1) {
					pcap_perror(handle,0);
				}

				printf("(Sent %d bytes)\n", sent);

			} else if (dest_ip == relayer_ip) {
				// If it comes to the relayer, just pass it to our attacker
				printf("Got an IP packet directed to our relayer from %s. Relaying to attacker... ",ip_dest_str);

				// Update the fields for replaying this packet
				//
				//	WARNING: We may have to update the checksum!
				memcpy((void *)ethernet->ether_shost, (void *)victim_mac, sizeof(ethernet->ether_shost));
				memcpy((void *)ethernet->ether_dhost, (void *)attacker_mac, sizeof(ethernet->ether_dhost));

				memcpy((void *)&(ip->ip_src.s_addr), (void *)&victim_ip, sizeof(ip->ip_src.s_addr));
				memcpy((void *)&(ip->ip_dst.s_addr), (void *)&attacker_ip, sizeof(ip->ip_dst.s_addr));

				// replay this packet to it's source from our relayer
				int sent = 0;
				if ((sent = pcap_inject(handle,packet,header.len)) == -1) {
					pcap_perror(handle,0);
				}

				printf("(Sent %d bytes)\n", sent);
			} // End if victim or relayer
		} // end ip
	} // end while
	pcap_close(handle);

}


int main (int argc, char **argv) {
  struct arguments arguments;

  /* Default values. */
  arguments.verbose = 0;
  arguments.victim_ip = "192.168.1.11";
  arguments.victim_mac = "00:0A:0B:0C:11:37";
  arguments.relayer_ip = "192.168.1.9";
  arguments.relayer_mac = "00:0A:06:1B:AB:B0";
  arguments.iface = "eth0";

  /* Parse our arguments; every option seen by parse_opt will
     be reflected in arguments. */
  argp_parse (&argp, argc, argv, 0, 0, &arguments);

	printf ("iface = %s\nvictim_ip = %s\nvictim_mac = %s\nrelayer_ip = %s\n"
		  "relayer_mac = %s\nVerbose = %s\n",
		  arguments.iface,
		  arguments.victim_ip, arguments.victim_mac,
		  arguments.relayer_ip,
		  arguments.relayer_mac,
		  arguments.verbose ? "yes" : "no");

	// Convert mac address to bytes
	unsigned char victim_mac[6];
	unsigned char relayer_mac[6];
	sscanf(arguments.victim_mac, "%x:%x:%x:%x:%x:%x",
	           (unsigned int *)&victim_mac[0],
			   (unsigned int *)&victim_mac[1],
			   (unsigned int *)&victim_mac[2],
			   (unsigned int *)&victim_mac[3],
			   (unsigned int *)&victim_mac[4],
			   (unsigned int *)&victim_mac[5]);
	sscanf(arguments.relayer_mac, "%x:%x:%x:%x:%x:%x",
				(unsigned int *)&relayer_mac[0],
				(unsigned int *)&relayer_mac[1],
				(unsigned int *)&relayer_mac[2],
				(unsigned int *)&relayer_mac[3],
				(unsigned int *)&relayer_mac[4],
				(unsigned int *)&relayer_mac[5]);

	// Convert IPs to ints
	in_addr_t victim_ip, relayer_ip;
	inet_pton(AF_INET, arguments.victim_ip, &victim_ip);
	inet_pton(AF_INET, arguments.relayer_ip, &relayer_ip);

	// Start our reflector
	reflect_packets(victim_ip,
			victim_mac,
			relayer_ip,
			relayer_mac,
			arguments.iface);

	exit (0);
}
