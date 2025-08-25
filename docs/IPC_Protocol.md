# AetherVisor: IPC Protocol Specification

## 1. Overview

The Inter-Process Communication (IPC) protocol enables secure and efficient communication between the C# WPF Frontend and the C++ Backend. A reliable, message-based system is required to handle asynchronous events like script execution requests and real-time console output.

**Chosen Technology:** Named Pipes.
- **Rationale:** Named pipes are a robust and well-supported IPC mechanism on Windows. They provide a reliable, connection-oriented, and byte-stream-based communication channel that can be secured with Windows access control lists (ACLs). They are simpler to implement securely than shared memory for this use case.

## 2. Pipe Naming

A unique, non-obvious pipe name will be used to prevent trivial detection or hijacking by other processes. The name will be algorithmically generated at runtime but will be consistent for a single AetherVisor session.

- **Format:** `\\.\pipe\AetherVisor_Session_{UUID}`
- **`{UUID}`:** A unique identifier generated when the backend starts. The frontend will need a mechanism (e.g., a temporary file or a command-line argument) to discover this name.

## 3. Message Structure

All communication will use a simple header-payload structure to define the message type and content.

```cpp
// C++ Representation
enum class MessageType : uint8_t {
    // Frontend -> Backend
    F2B_HANDSHAKE_REQ = 0x01,
    F2B_EXECUTE_SCRIPT = 0x02,
    F2B_SHUTDOWN_REQ = 0x03,

    // Backend -> Frontend
    B2F_HANDSHAKE_ACK = 0x81,
    B2F_CONSOLE_OUTPUT = 0x82,
    B2F_STATUS_UPDATE = 0x83, // e.g., "Injected", "Target Closed"
};

struct MessageHeader {
    MessageType type;
    uint32_t payloadSize;
};

// A full message is [MessageHeader][PayloadData]
```

- **`type`:** An 8-bit enum defining the purpose of the message.
- **`payloadSize`:** A 32-bit integer indicating the size of the data that follows the header.
- **`PayloadData`:** A variable-length byte array containing the actual data (e.g., UTF-8 encoded script or console string).

## 4. Communication Flow

1.  **Initialization:**
    *   Backend starts and creates the named pipe server instance.
    *   Frontend starts and attempts to connect to the named pipe.

2.  **Handshake:**
    *   `Frontend -> Backend`: Sends `F2B_HANDSHAKE_REQ`.
    *   `Backend -> Frontend`: Responds with `B2F_HANDSHAKE_ACK` to confirm the connection is valid.

3.  **Script Execution:**
    *   `Frontend -> Backend`: Sends `F2B_EXECUTE_SCRIPT` with the Luau script as the payload.
    *   The backend receives the message, extracts the script, and forwards it to the injected DLL for execution.

4.  **Console Logging:**
    *   The injected DLL in Roblox hooks a console function (`print`, `warn`, etc.).
    *   When the function is called, the hook captures the output string.
    *   The DLL sends the string to the Backend.
    *   `Backend -> Frontend`: The backend relays this string by sending a `B2F_CONSOLE_OUTPUT` message with the log string as the payload. The frontend UI updates the console panel.

5.  **Shutdown:**
    *   When the user closes the frontend UI:
    *   `Frontend -> Backend`: Sends `F2B_SHUTDOWN_REQ`.
    *   The backend initiates the full `Cleanup` procedure.

## 5. Security Considerations

- **ACLs:** The named pipe will be created with an Access Control List (ACL) that restricts access to only the user account running AetherVisor. This prevents other unelevated processes from interfering.
- **Message Validation:** The backend and frontend will validate all incoming messages. Any message with an unknown `MessageType` or an inconsistent `payloadSize` will be discarded, and the connection may be terminated.
- **No Sensitive Data:** The IPC channel will not be used to transmit highly sensitive data like encryption keys or user credentials. Its purpose is limited to operational commands and logs.
