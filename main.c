#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s -P <ip:port>... -E <program>\n", argv[0]);
        return 1;
    }

    char *program = NULL;
    char **ip_ports = NULL;
    int ip_ports_count = 0;
    char **program_args = NULL;
    int program_args_count = 0;

    char *timeout = NULL;      // -T
    char *timeout_mil = NULL;  // -M

    int max_retries = 3;
    int thread_sleep_count = 3;

    bool any_connected = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-P") == 0) {
            ip_ports = &argv[i + 1];
            while (i + 1 < argc && argv[i + 1][0] != '-') {
                ip_ports_count++;
                i++;
            }
        } else if (strcmp(argv[i], "-E") == 0 && i + 1 < argc) {
            program = argv[i + 1];
            program_args = &argv[i + 1];
            while (i + 1 < argc && argv[i + 1][0] != '-') {
                program_args_count++;
                i++;
            }
        } else if (strcmp(argv[i], "-T") == 0 && i + 1 < argc) {
            timeout = argv[i + 1];
            max_retries = atoi(timeout);
        } else if (strcmp(argv[i], "-M") == 0 && i + 1 < argc) {
            timeout_mil = argv[i + 1];
            thread_sleep_count = atoi(timeout_mil);
        }
    }

    // check -P -E should not be null
    if (program == NULL || ip_ports == NULL) {
        printf("Invalid arguments: Missing -P or -E.\n");
        return 1;
    }

    for (int i = 0; i < ip_ports_count; i++) {
        int retry_count = 0;

        char *ip_port = ip_ports[i];
        char *delimiter_pos = strchr(ip_port, ':');
        if (delimiter_pos == NULL) {
            printf("Invalid IP:port format: %s\n", ip_port);
            return 1;
        }

        *delimiter_pos = '\0';
        char *ip = ip_port;
        int port = atoi(delimiter_pos + 1);

        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("socket creation failed");
            return 1;
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        if (inet_aton(ip, &addr.sin_addr) == 0) {
            struct hostent *he = gethostbyname(ip);
            if (he == NULL) {
                printf("Cannot resolve domain: %s\n", ip);
                close(sock);
                continue; // try next IP:Port
            }
            addr.sin_addr = *((struct in_addr *)he->h_addr);
            printf("Resolved domain %s, IP Address: %s\n", ip, inet_ntoa(addr.sin_addr));
        }

        printf("Trying to connect to %s:%d\n", ip, port);

        bool connected = false;
        while (retry_count < max_retries) {
            if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
                printf("Connection to %s:%d succeeded.\n", ip, port);
                connected = true;
                any_connected = true;
                break;
            } else {
                perror("Failed to connect");
                retry_count++;
                if (retry_count < max_retries) {
                    printf("Retrying in %d seconds...\n", thread_sleep_count);
                    sleep(thread_sleep_count);
                }
            }
        }

        close(sock);
        
        if (connected) {
            // break;
        }
    }
    
    if (any_connected) {
        execvp(program, program_args);
        perror("execvp failed");
        return 1;
    } else {
        printf("All connections failed. Program will not run.\n");
    }

    return 0;
}
