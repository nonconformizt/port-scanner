// gcc main.c -o scan.exe -lresolv -pthread
/**
 * Better make this a dynamic hash table
 */

#define TBL_SIZE 3400
char * port_assoc[TBL_SIZE] = {0};

void fill_port_assoc () 
{
    port_assoc[1] = "TCPMUX";
    port_assoc[5] = "RJE";
    port_assoc[7] = "ECHO";
    port_assoc[18] = "MSP";
    port_assoc[20] = "FTP Data";
    port_assoc[21] = "FTP Control";
    port_assoc[22] = "SSH";
    port_assoc[23] = "Telnet";
    port_assoc[25] = "SMTP";
    port_assoc[29] = "MSP";
    port_assoc[37] = "Time";
    port_assoc[42] = "Host Name Server";
    port_assoc[43] = "Whois";
    port_assoc[49] = "Login Host Protocol";
    port_assoc[53] = "DNS";
    port_assoc[69] = "TFTP";
    port_assoc[70] = "Gopher";
    port_assoc[79] = "Finger";
    port_assoc[80] = "HTTP";
    port_assoc[103] = "X.400";
    port_assoc[108] = "SNA Gateway Access Server";
    port_assoc[109] = "POP2";
    port_assoc[110] = "POP3";
    port_assoc[115] = "SFTP";
    port_assoc[118] = "SQL";
    port_assoc[119] = "NNTP";
    port_assoc[137] = "NetBIOS Name Service";
    port_assoc[139] = "NetBIOS Datagram Service";
    port_assoc[143] = "IMAP";
    port_assoc[150] = "NetBIOS Session Service";
    port_assoc[156] = "SQL Server";
    port_assoc[161] = "SNMP";
    port_assoc[179] = "BGP";
    port_assoc[190] = "GACP";
    port_assoc[194] = "IRC";
    port_assoc[197] = "DLS";
    port_assoc[389] = "LDAP";
    port_assoc[443] = "HTTPS";
    port_assoc[444] = "SNPP";
    port_assoc[445] = "Microsoft-DC";
    port_assoc[458] = "Apple QuickTime";
    port_assoc[546] = "DHCP Client";
    port_assoc[547] = "DHCP Server";
    port_assoc[569] = "MSN";
    port_assoc[1080] = "Socks";
    port_assoc[3306] = "Mysql";
}