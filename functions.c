#include "header.h"



void scan_host(struct in_addr target, int port_lo, int port_hi, int socket_fd);
void * listen_host(void *target);
void fatal_err(const char * msg);
int parse_cidr(const char* cidr, struct in_addr* addr, struct in_addr* mask);
void parse_args(int argc, char *argv[], struct in_addr *addr, int *port_lo, int *port_hi, int * num_hosts);
void process_packet(unsigned char * buffer, int size, struct in_addr source_ip, struct in_addr dest_ip);
const char* dotted_quad(const struct in_addr* addr);
void prepare_datagram(char* datagram, struct iphdr* iph, struct tcphdr* tcph, struct in_addr dest_ip);
void ip_to_host(const char* ip, char* buffer);
void get_local_ip(char* buffer);
unsigned short check_sum(unsigned short* ptr, int nbytes);



void scan_host(struct in_addr target, int port_lo, int port_hi, int socket_fd)
{    
    // create thread for listening 
    pthread_t listen_thread;
    pthread_create(&listen_thread, NULL, listen_host, (void *)(&target));

    char source_ip[INET6_ADDRSTRLEN];
    get_local_ip(source_ip);

    struct in_addr dest_ip;
    dest_ip.s_addr = inet_addr(dotted_quad(&target));
    
    char datagram[DATAGRAM_SIZE];
    struct iphdr* iph = (struct iphdr*)datagram; //IP header
    struct tcphdr* tcph = (struct tcphdr*)(datagram + sizeof(struct ip)); //TCP header

    prepare_datagram(datagram, iph, tcph, dest_ip);


    // for each port generate and send packets from port_lo to port_hi
    for (int port = port_lo; port <= port_hi; port++)
    {
        struct sockaddr_in dest;
        struct pseudo_header psh;

        dest.sin_family = AF_INET;
        dest.sin_addr.s_addr = dest_ip.s_addr;

        tcph->dest = htons(port);
        tcph->check = 0;

        psh.source_address = inet_addr(source_ip);
        psh.dest_address = dest.sin_addr.s_addr;
        psh.placeholder = 0;
        psh.protocol = IPPROTO_TCP;
        psh.tcp_length = htons(sizeof(struct tcphdr));

        memcpy(&psh.tcp, tcph, sizeof(struct tcphdr));

        tcph->check = check_sum((unsigned short*)&psh, sizeof(struct pseudo_header));

        if (sendto(socket_fd, datagram, sizeof(struct iphdr) + sizeof(struct tcphdr), 0, (struct sockaddr*)&dest, sizeof(dest)) < 0)
            fatal_err("Error sending syn packet!");

    }

    sleep(WAIT_TIMEOUT);
    pthread_cancel(listen_thread); 

    printf("\n");
}

void * listen_host(void* target)
{
    struct in_addr * target_in = (struct in_addr*) target;
    unsigned char* buffer = (unsigned char*) malloc(BUF_SIZE);

    socklen_t saddr_size, data_size;
    struct sockaddr_in saddr;

    // create listening raw socket
    int sock_raw = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock_raw < 0) 
        fatal_err("Cannot create listening socket!");


    printf("%s Open ports: ", inet_ntoa(*target_in));

    /** 
     * The ideal approach is to wait
     * until timeout OR until we receive responce
     * from ALL requested ports of target 
     * 
     * BUT here I will just kill the thread after timeout
     */

    while(1)
    {
        data_size = recvfrom(sock_raw, buffer, BUF_SIZE, 0, (struct sockaddr*) &saddr, &saddr_size);
        if (data_size < 0)
            fatal_err("Recvfrom error, failed to get packets!");
        
        process_packet(buffer, data_size, saddr.sin_addr, *target_in);
    }

}


void fatal_err(const char * msg)
{
    printf("\033[0;31m");
    printf("[Error]: %s\n", msg);
    printf("\033[0m");
    exit(1);
}


/**
  Parse CIDR notation address.
  Return the number of bits in the netmask if the string is valid.
  Return -1 if the string is invalid.
 */
int parse_cidr(const char* cidr, struct in_addr* addr, struct in_addr* mask)
{
    int bits = inet_net_pton(AF_INET, cidr, addr, sizeof addr);

    mask->s_addr = htonl(~(bits == 32 ? 0 : ~0U >> bits));
    return bits;
}

/**
 * Parse command-line arguments
 */
void parse_args(
    int argc, char * argv[], 
    struct in_addr *addr, 
    int *port_lo, int *port_hi,
    int * num_hosts)
{

    if (argc <= 1)
        fatal_err("Too few arguments!");
    
    struct in_addr parsed_addr, mask, 
                   wildcard_addr, network_addr, 
                   broadcast_addr, min_addr, max_addr;

    if (parse_cidr(argv[1], &parsed_addr, &mask) < 0)
        fatal_err("Invalid target address!");
    // make some weird stuff i don't understand
    {
        wildcard_addr = mask;
        wildcard_addr.s_addr = ~wildcard_addr.s_addr;

        network_addr = parsed_addr;
        network_addr.s_addr &= mask.s_addr;

        broadcast_addr = parsed_addr;
        broadcast_addr.s_addr |= wildcard_addr.s_addr;

        min_addr = network_addr;
        max_addr = broadcast_addr;

        if (network_addr.s_addr != broadcast_addr.s_addr) {
            min_addr.s_addr = htonl(ntohl(min_addr.s_addr) + 1);
            max_addr.s_addr = htonl(ntohl(max_addr.s_addr) - 1);
        }

        *addr = min_addr;
        *num_hosts = (int64_t)ntohl(broadcast_addr.s_addr) - ntohl(network_addr.s_addr) + 1;
    }


    if (argc <= 2) {
        // port range not specified
        *port_lo = PORT_LO;
        *port_hi = PORT_HI;
    }
    else {
        char *ptr = strtok(argv[2], "-");
        if (ptr == NULL)
            fatal_err("Invalid port!");
        *port_lo = abs(atoi(ptr));
        ptr = strtok(NULL, "-");
        if (ptr == NULL)
            fatal_err("Invalid port!");
        *port_hi = abs(atoi(ptr));
        if (*port_hi < *port_lo) {
            int tmp = *port_lo;
            *port_lo = *port_hi;
            *port_hi = tmp;
        }
    }

}

/**
  Method to process incoming packets and look for Ack replies
*/
void process_packet(
    unsigned char * buffer, 
    int size, 
    struct in_addr source_ip, 
    struct in_addr dest_ip)
{
    struct iphdr* iph = (struct iphdr*) buffer; // IP Header part of this packet
    unsigned short iphdrlen;
    struct sockaddr_in source, dest;
    memset(&dest, 0, sizeof(dest));
    memset(&source, 0, sizeof(source));


    if (iph->protocol == 6) 
    {        
        struct iphdr* iph = (struct iphdr*) buffer;
        iphdrlen = iph->ihl * 4;

        struct tcphdr* tcph = (struct tcphdr*)(buffer + iphdrlen);

        source.sin_addr.s_addr = iph->saddr;

        dest.sin_addr.s_addr = iph->daddr;

        if (tcph->syn == 1 && tcph->ack == 1 && source.sin_addr.s_addr == dest_ip.s_addr) {
            printf("%d ", ntohs(tcph->source));
            fflush(stdout);
        }
    }
}


/**
  Initialize the datagram packet
 */
void prepare_datagram(char* datagram, struct iphdr* iph, struct tcphdr* tcph, struct in_addr dest_ip)
{
    memset(datagram, 0, DATAGRAM_SIZE);

    char source_ip[INET6_ADDRSTRLEN];
    get_local_ip(source_ip); 

    //Fill in the IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct ip) + sizeof(struct tcphdr);
    iph->id = htons(46156); //Id of this packet
    iph->frag_off = htons(16384);
    iph->ttl = 64;
    iph->protocol = IPPROTO_TCP;
    iph->check = 0; //Set to 0 before calculating checksum
    iph->saddr = inet_addr(source_ip); //Spoof the source ip address
    iph->daddr = dest_ip.s_addr;
    iph->check = check_sum((unsigned short*)datagram, iph->tot_len >> 1);

    //TCP Header
    tcph->source = htons(46156); //Source Port
    tcph->dest = htons(80);
    tcph->seq = htonl(1105024978);
    tcph->ack_seq = 0;
    tcph->doff = sizeof(struct tcphdr) / 4; //Size of tcp header
    tcph->fin = 0;
    tcph->syn = 1;
    tcph->rst = 0;
    tcph->psh = 0;
    tcph->ack = 0;
    tcph->urg = 0;
    tcph->window = htons(14600); //Maximum allowed window size
    tcph->check = 0; //If you set a checksum to zero, your kernel's IP stack should fill in the correct checksum during transmission
    tcph->urg_ptr = 0;
}

/**
  Format the IPv4 address in dotted quad notation, using a static buffer.
 */
const char* dotted_quad(const struct in_addr* addr)
{
    static char buf[INET_ADDRSTRLEN];

    return inet_ntop(AF_INET, addr, buf, sizeof buf);
}

/**
 Get hostname of an IP address
 */
void ip_to_host(const char* ip, char* buffer)
{
    struct sockaddr_in dest;

    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr(ip);
    dest.sin_port = 0;

    if (getnameinfo((struct sockaddr*)&dest, sizeof(dest), buffer, NI_MAXHOST, NULL, 0, NI_NAMEREQD) != 0)
        strcpy(buffer, "Hostname can't be determined");
}

/**
 Get source IP of the system running this program
 */
void get_local_ip(char* buffer)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    const char* kGoogleDnsIp = "8.8.8.8";
    int dns_port = 53;

    struct sockaddr_in serv;

    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(kGoogleDnsIp);
    serv.sin_port = htons(dns_port);

    if (connect(sock, (const struct sockaddr*)&serv, sizeof(serv)) != 0)
        fatal_err("Failed to get local IP!");

    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);

    if (getsockname(sock, (struct sockaddr*)&name, &namelen) != 0)
        fatal_err("Failed to get local IP!");

    inet_ntop(AF_INET, &name.sin_addr, buffer, INET6_ADDRSTRLEN);

    close(sock);
}

/**
 Checksums - IP and TCP
 */
unsigned short check_sum(unsigned short* ptr, int nbytes)
{
    register long sum;
    register short answer;
    unsigned short oddbyte;

    sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }

    if (nbytes == 1) {
        oddbyte = 0;
        *((u_char*)&oddbyte) = *(u_char*)ptr;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum = sum + (sum >> 16);
    answer = (short)~sum;

    return answer;
}
