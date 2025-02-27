# Learning With Errors-based Public Key Encryption Key Generation in seL4
This repository is part of PhD research.  It demonstrates using the microkit API to create an isolated key management system.  

## Protection Domains
We have developed three (3) protection domains (PD): 
1. (PD1) key generation request (client.c): this client provides no functionality either than to use the notify() entry point to the key generating server in order to start the key generating process. 
2. (PD2) key generating server (server.c): the server begins generating asymmetric keys based on a learning with errors (LWE) public key encryption scheme.  The keys are generated according to the mathematics described below.  After generationg, they are written to a virtual memory space that is shared with the key consumer.  Thereafter, the server uses the notify() entry point to tell the key consumer that the keys are ready.
3. (PD3) key consumer (consumer.c): the consumer waits until it is notified by the key generation server.  Once notified, the consumer reads from the shared memory space and prints the keys.

```mermaid
graph LR
A[PD1] -- notify() --> B[PD2] 
B -- Produces Key (RW) --> C[Shared Memory]
C -- Consumes Key (R) --> D[PD3]
```
