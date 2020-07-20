# Port scanner 

Here you can find a simple port-scanner for Linux, just like NMap, but tiny.
This is an educational project, which I wrote as a networks task for university.

This scanner uses TCP SYN technique to distinguish between open and closed ports. You may learn more about scan techniques [here](https://nmap.org/book/man-port-scanning-techniques.html)

It can scan just one host, or a specified subnet, if you want to.

Compile: `gcc main.c -o scanner -lresolv -pthread`

## Usage

Since in UNIX systems (including Linux) you must have user priveleges, therefore you have to run scanner with sudo.

- `sudo ./scanner 192.168.10.5 1000-3000` - scan ports 1000 to 3000 of `192.168.10.5`
- `sudo ./scanner 192.168.10.0/24 123-321` - scan ports 123 to 321 of subnet `192.168.10.0/24` (IPs from `192.168.10.0` to `192.168.10.255`)
- `sudo ./scanner 192.168.10.0/24 123-321 -d` - `-d` flag means services detection (scanner will try to find each open port in association table)

## Architecture

Scanner is multithreaded. At a startup it creates some number of threads, but not greater than the number of hosts to scan, and not greater than `MAX_THREADS` constant, specified in the header file. After that scanner fills up a stack of addresses to scan (like a list of jobs to be done). 
Each working thread makes one listening thread, and than picks up one host address from stack, than creates and send raw SYN-packets, one packet for each port in a scanning range. After that working thread waits for some timeout, stops its listening thread and prints out listening thread's report about received packets.

## Contribution

I am intrested in further development of this scanner. Feel free to improve this project and to add some new features. I will check all pull requests, etc.

At the first place, this project needs proper Make file. Some code needs to be split up into separate modules. Also I don't like current cli arguments parsing (user can specify only port range, not just one port, and not arbitrary set of ports). Port association table is horrible, I would like to make a hash table instead of just array.

You can email me (sceptlc@bk.ru) and suggest any improvements :)
