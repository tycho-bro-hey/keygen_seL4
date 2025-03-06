# Learning With Errors-based Public Key Encryption Key Generation in seL4
This repository is part of PhD research.  It demonstrates using the microkit API to create an isolated key management system.  

## Protection Domains
We have developed three (4) protection domains (PD): 
1. (PD1) key generation request (client.c): this client provides no functionality either than to use the notify() entry point to the key generating server in order to start the key generating process. This protection domain will be removed at a later date.
2. (PD2) key generating server (server.c): the server begins generating asymmetric keys based on a learning with errors (LWE) public key encryption scheme.  The keys are generated according to the mathematics described below.  After generationg, they are written to a virtual memory space that is shared with the key consumer.  Thereafter, the server uses the notify() entry point to tell the key consumer that the keys are ready.
3. (PD3) public key consumer (pk_consumer.c): the public key consumer waits until it is notified by the key generation server.  Once notified, the public key consumer reads from the shared memory space and prints the public key.
4. (PD4) secret key consumer (sk_consumer.c): the secret key consumer waits until it is notified by the key generation server.  Once notified, the secret key consumer reads from the shared memory space and prints the secret key.

```mermaid
graph TD;
    A[Key Generation] -->|produces secret key<br/>(read & write)| B[Shared Memory<br/>private key]
    A --> C[Shared Memory<br/>secret key]
    B --> D[Encrypt]
    C --> E[Decrypt]

    style A fill:#2196F3,stroke:#1E88E5,color:#FFFFFF
    style B fill:#B71C1C,stroke:#880E4F,color:#FFFFFF
    style C fill:#B71C1C,stroke:#880E4F,color:#FFFFFF
    style D fill:#2196F3,stroke:#1E88E5,color:#FFFFFF
    style E fill:#2196F3,stroke:#1E88E5,color:#FFFFFF

```
