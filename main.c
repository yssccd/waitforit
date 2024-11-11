#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

    char *timeout = NULL;
    char *timeout_mil = NULL;

    int max_retries = 3;
    int thread_sleep_count = 3;
    int retry_count = 0;
    bool connected = false;

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
        } else if (strcmp(argv[i], "-M") == 0 && i + 1 <argc){
            timeout_mil = argv[i + 1];
            thread_sleep_count = atoi(timeout_mil);
        }
    }

    if (program == NULL || ip_ports == NULL) {
        printf("Invalid arguments\n");
        return 1;
    }

    for (int i = 0; i < ip_ports_count; i++) {
        char *ip_port = ip_ports[i];
        char *delimiter_pos = strchr(ip_port, ':');
        if (delimiter_pos == NULL) {
            printf("Invalid IP:port format\n");
            return 1;
        }

        *delimiter_pos = '\0';
        char *ip = ip_port;
        int port = atoi(delimiter_pos + 1);

        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_aton(ip, &addr.sin_addr);

        // print the ip and port
        printf("Trying to connect to %s:%d\n", ip, port);

        while (retry_count < max_retries) {
            if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
                connected = true;
                break;
            } else {
                retry_count++;
                connected = false;
                perror("Failed to connect");
                printf("Retrying in 3 seconds...\n");
                sleep(thread_sleep_count);
            }
        }
        close(sock);
    }

    if (connected) {
        execvp(program, program_args);
    }
    return 0;
}