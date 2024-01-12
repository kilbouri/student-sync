# Student-Sync Protocol

This document outlines the protocol of communications for the Student-Sync software.

## Message Types

Student-Sync uses Tag-Length-Value (TLV) encoded messaging. This is chosen as it provides a relatively small overhead 
while also enabling the transmission of raw binary data, without needing to encode it to a text-based format (such as Base64).

The structure of a single message is as follows:

```
Tag		(1 byte)
Length	(8 bytes)
Value	(Length bytes)
```

**Note**: the list of tags and their specific values are currently subject to change without warning.

## Communication

Below outlines the flow of communication between the Client and Server. If at any point either party
does not send an expected reply within an implementation-defined timeout (recommended minimum of 1 minute 
on the server, and 30 seconds on the client), then the connection MAY be closed immediately. Likewise, if
at any point a reply is received with an incorrect tag or invalid data, the implementor MAY choose to send
a `Error` message with a reason, then MUST close the connection. If no `Error` message is sent, the connection
MUST still be closed.

Assume the server is running on `127.0.0.1:1000`. The client will connect to the server 
at `127.0.0.1:1000`. If the server accepts the connection, the following communication will
occur **immediately**:

1. Client: `Hello`, carrying the client's configured username
2. Server: `Ok`, if the server successfully registers the client.

If/when the server operator decides to view the screen of the client, the following communication
will occur:

1. Server: `GetStreamParams`
2. Client: `StreamParams`, carrying the client's configured stream parameters (frame rate, resolution)
3. Server: `InitializeStream`, carrying stream parameters that are AT MOST the client's requested parameters. The parameters MAY be lower.
4. Client: `StreamFrame`, carrying a single frame of the stream, of resolution not exceeding that received in the preceeding `InitializeStream` message.
5. Continue from 4 once 1000/FPS milliseconds have passed since the last `StreamFrame` message was sent.
6. Server, at any time after `InitializeStream`: `EndStream` at which point the client must break out of the loop above.

Implementation-specific communication may occur when the above is not in progress. Note, however, that this extra communication may be ignored
or cause the other party to close the connection if they are not expecting such communication.