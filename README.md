# Learning With Errors-based Public Key Encryption Key Generation in seL4
This repository is part of PhD research.  It demonstrates using the microkit API to create an isolated key management system.  

## Protection Domains
We have developed three (3) protection domains (PD): 
1. (PD1) key generation request (client.c), 
2. (PD2) key generating server (server.c)
3. (PD3) key consumer (consumer.c).
