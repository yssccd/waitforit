# waitforit

Wait for TCP/IP port to be available before executing a command.

## How to run

1. Build it:
    ```sh
    cmake .
    make
    ```

2. Then you can use it. Example:
    ```sh
    ./wait_process_c -P 192.168.1.2:80 192.168.2.2:443 -E ls
    ./wait_process_c -P 192.168.1.2:80 -E ls
    ```

## Known issue

Currently does not support domain names; only IP addresses can be used.