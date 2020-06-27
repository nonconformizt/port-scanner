#if !defined(HEADER)
#define HEADER

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


#define PORT_LO 1 
#define PORT_HI 65535 

#define BUF_SIZE 65536
#define DATAGRAM_SIZE 4096

// how long to wait for host to respond
#define WAIT_TIMEOUT 3

// default string length
#define STR_SIZE 1024

struct pseudo_header { //Needed for checksum calculation
    unsigned int source_address;
    unsigned int dest_address;
    unsigned char placeholder;
    unsigned char protocol;
    unsigned short tcp_length;

    struct tcphdr tcp;
};

// Global variables
int socket_fd,
    port_lo,
    port_hi;


// Stack for unresolved ip-adresses
pthread_mutex_t stack_lock;
struct in_addr * stack; // array of addresses
int stack_size;
int stack_top;



#endif // HEADER
