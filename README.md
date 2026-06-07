# winsock-file-transfer
C++ client-server file transfer application built with Winsock over TCP using a custom packet header supporting multiple sequential transfers. 


# Youtube video including client/server code overview with demo

https://youtu.be/_Y3NWa8Z8kM



## Overview

This project is a custom file transfer application built in C++ using Winsock. It transfers files over a TCP stream using a custom packet header in a client-server architecture. The application supports multiple file transfers sequentially within the same session, but it does not support simultaneous multi-client transfers.






## Features

- custom file headers with protocol, type, size and filename fields
- binary-safe file transfer
- support for multiple sequential file transfers in one session
- retry logic for invalid client input and failed connection attempts
- graceful quit packet handling 
- file reconstruction and validation on server side






## How It Works (Client side)

The client begins by initializing Winsock and creating a TCP socket. It prompts the user to enter a server IP address, validates the address format, and keeps retrying until a successful connection is established. Once connected, the client prompts the user for a filename, attempts to open the file in binary mode, and reads its contents into memory. It then builds a custom packet header containing the protocol value, packet type, file size, and filename, sends that header to the server, and sends the file payload over the TCP connection using an exact-send helper. After each transfer, the client allows the user to send another file or quit the session, then closes the socket and cleans up Winsock resources when the session ends.





## How It Works (Server side)

The server begins by initializing Winsock, creating a listening TCP socket, binding it to port 9090, and waiting for an incoming client connection. Once a client connects, the server accepts the connection and enters a loop that reads a custom packet header from the TCP stream using the recv_exact helper. It validates the protocol value, checks the packet type, and if the packet is a file transfer, uses the size field from the header to receive the exact number of payload bytes expected. The server then writes the received binary data to disk using the filename stored in the header, confirms that the full file was written successfully, and continues waiting for additional files in the same session. If the client disconnects, sends a quit packet, or a transfer error occurs, the server exits the session loop, closes the client and listening sockets, and cleans up Winsock resources.






## Packet/Header Format

Each file transfer begins with a custom packet header sent before the file payload. This header allows the receiver to determine whether the packet belongs to this custom protocol, what type of data is being sent, how many bytes to expect, and what filename should be used when saving the file.

- protocol (uint32_t) — validates that the incoming packet belongs to this custom protocol. In this project, the value 42 is used.
- type (uint32_t) — identifies the type of packet being sent. 2 is used for file transfers and 3 is used for quit packets.
- size (uint64_t) — stores the exact number of payload bytes expected so the receiver knows how many bytes must be read from the TCP stream.
- filename (char[MAX_FILENAME]) — stores the filename so the server knows what name to use when reconstructing and saving the received file.







## Why send\_exact / recv\_exact functions were needed

TCP is a stream-based protocol, which means data is delivered as a continuous flow of bytes rather than as complete messages. Because of this, a single call to the Winsock send() and recv() functions can sometimes work, but it is inconsistent and cannot be relied on to transfer an entire buffer in one operation. For example, if the client tries to send a 1,000-byte file payload, one send() call might transmit all 1,000 bytes in one test, but in another case it might only send 400 bytes, while the server’s first recv() call might only read 200 bytes even though more data is still on the way. The send\_exact and recv\_exact helper functions were needed to repeatedly call send() and recv() until the exact number of expected bytes had been fully transmitted or received, ensuring the custom header and binary file payload were transferred correctly without truncation or corruption.







## How to Run


### Requirements
- Windows machine with Visual Studio and Winsock support
- Both the client and server must be able to reach each other over the network
- By default, the server listens on TCP port `9090`

### Network Requirement
For the project to work in its current form, the client must be able to directly reach the server’s IP address over the network. In most normal test setups, that means both devices should be on the same LAN or on networks/VLANs that can route traffic to each other and allow TCP traffic over port `9090`.

### Server
- Open the server project in Visual Studio.
- Build and run the server.
- The server initializes Winsock, binds to port `9090`, and begins listening for an incoming client connection.

### Client
- Open the client project in Visual Studio.
- Build and run the client.
- Enter the server’s IPv4 address when prompted.
- Once connected, enter the name of the file you want to transfer (file must be in the working directory).
- Type 'YES' to send the file or 'QUIT' to cancel the session.
- After a successful transfer, the client can continue sending additional files in the same session.

### Expected Behavior
- The client sends a custom header first, followed by the file payload.
- The server reads the header, validates the protocol and packet type, receives the expected number of bytes, and writes the file to disk.
- The session remains open so multiple files can be transferred without reconnecting each time.








## Troubleshooting

During development, I encountered an environment-specific Windows issue where the client could successfully ping the target machine, but TCP connection attempts still failed. After testing firewall rules and other connectivity checks, I discovered that a third-party network service on my development machine was interfering with TCP connections. Stopping the service resolved the issue and allowed the client to connect to the server normally.

command used to stop the service: 'Stop-Service nimDNSResponder -Force'







## Example Workflow 

- Start the server and wait for it to begin listening on port 9090.
- Start the client and enter the server’s IPv4 address.
- Enter the name of the file you want to transfer.
- Type YES to confirm the transfer.
- The client sends the custom header followed by the file payload.
- The server receives the header, reads the expected number of payload bytes, and writes the file to disk.
- The session remains open so additional files can be transferred or the client can quit.







## What I learned

This project allowed me to develop and strengthen several important skills in C++ network programming. I learned how to initialize Winsock, create and bind TCP sockets, and build a client-server application that transfers binary data reliably over a TCP stream. I also gained experience designing a custom packet header and writing helper functions such as send_exact and recv_exact to work around the inconsistent behavior of single send() and recv() calls in a stream based protocol. In addition, this project helped me improve my understanding of pointers, file I/O, and how to read and apply Microsoft API documentation while building a real working system.





## Future Improvements

- Add file hash based verification to confirm that received files match the original transmitted data.
- Add encryption so file transfers are protected in transit instead of being sent unencrypted over the network.
- Expand support for use beyond a single local network by improving handling for routed or remote network environments.
- Add support for multiple simultaneous client connections instead of sequential single-client sessions.
- Add a login/authentication system so that clients can retrieve files stored on the server



