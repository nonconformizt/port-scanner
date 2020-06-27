#include "header.h"
#include "functions.c"


int main (int argc, char * argv[])
{
    struct in_addr addr;
    int port_lo, port_hi, num_hosts;

    parse_args(argc, argv, &addr, &num_hosts);

    fill_stack(addr, num_hosts);

    socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (socket_fd < 0)
        fatal_err("Error creating socket!");


    // Set IP_HDRINCL socket option to tell the kernel 
    // that headers are included in the packet
    int oneVal = 1;
    if (setsockopt(socket_fd, IPPROTO_IP, IP_HDRINCL, &oneVal, sizeof(oneVal)) < 0)
        fatal_err("Error setting IP_HDRINCL!");


    // create working threads
    pthread_t threads[num_hosts];

    for (int i = 0; i < num_hosts; i++)
    {
        pthread_create(&threads[i], NULL, scan_host, NULL);
    }

    for (int i = 0; i < num_hosts; i++)
    {
        pthread_join(threads[i], NULL);
    }


    scan_host();

}

