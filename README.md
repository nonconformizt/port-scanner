# Port scanner 

Here you can find a simple port-scanner, just like NMap, but tiny.
This is an educational project, which I wrote as a networks task for university.

This scanner uses TCP SYN technique to distinguish between open and closed ports. You may learn more about scan techniques [here](https://nmap.org/book/man-port-scanning-techniques.html)

It can scan just one host, or a specified subnet, if you want to.

## Architecture

Scanner is multithreaded. At a startup it creates some number of threads, but not greater than the number of host to scan, and not greater than `MAX_THREADS` constant, specified in the header file. Also scanner fills up a special stack with addresses of hosts to scan. You may think of it as a list of jobs to be done. 
Each working thread makes one listening thread, and than picks up one host address from stack, than creates and send raw SYN-packets, one packet for each port in a scanning range. After that working thread waits for some timeout, stops its listening thread and prints out listening thread's report about received packets.
