# NCTU.107-2.unix-programming


## Project1
netstat -nap

### Grading Policy
* [10%] List TCP and UDP connetions (IPv4).
* [10%] List TCP and UDP connetions (IPv6).
* [30%] Show corresponding program names and arguments for each identified connection.
* [10%] Implement -u and --udp option using getopt\_long(3).
* [10%] Implement -t and --tcp option using getopt\_long(3).
* [10%] Translate network address into user-friendly formats, e.g., from `0100007F` to `127.0.0.1`, and from `FE01A8C0` to `192.168.1.254`.
* [10%] Implement basic filter string feature.
* [10%] Use Makefile to manage the building process of your program.
* [10%] If your filter string supports regular expression, see regex(3).

### Patch (84%)
* Fix parameters in inet\_ntop for IPv6 rmt ip address
  > -5
* Close opened dir
  > \* 0.8  
  > Cause Segmentation Fault
* Increase PROCESS\_NUM


## Project2
Library Call Monitor

### Grading Policy
* [10%] A monitored executable can work as usual. Your program cannot break the functions of a monitored executable.
* [20%] Monitor functions listed in minimum requirements.
* [20%] Provide basic list for function call parameters and return values.
* [20%] Provide comprehensive list for function call parameters and return values.
* [20%] Output can be configured using `MONITOR_OUTPUT` environmental variable.
* [10%] Use Makefile to manage the building process of your program. We will not grade your program if we cannot use make command to build your program.

### Patch (95%)
* Does not show `fscanf`


## Project3
Extend the Mini Lib C to Handle Signals

### Grading Policy
* [12%] alarm
* [12%] sigprocmask
* [12%] sigpending
* [12%] functions to handle sigset\_t data type
* [20%] setjmp and longjmp
* [20%] signal and sigaction
* [22%] extra (unpublished) test cases
