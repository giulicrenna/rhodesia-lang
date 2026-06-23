---
title: net
layout: default
nav_order: 12
parent: Standard Library
---

# Net Module

The `net` module provides TCP socket primitives and an HTTP/1.1 client built directly on POSIX sockets. No external dependencies are required.

> **Note:** HTTPS is not supported in this version (no TLS). All HTTP functions work only with `http://` URLs.

## Table of Contents

- [Constants](#constants)
- [Socket API](#socket-api)
- [HTTP API](#http-api)
- [Response Record](#response-record)
- [Examples](#examples)

---

## Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `net.DEFAULT_PORT_HTTP` | `80` | Default HTTP port |
| `net.DEFAULT_TIMEOUT` | `30` | Suggested timeout in seconds (informational) |

---

## Socket API

Sockets are identified by opaque integer handles returned by `net.socket()` or `net.accept()`.

---

### `net.socket(host, port)` → `int`

Create a TCP socket for the given host and port.

The socket is **not connected** after this call. Call `net.connect()` next.

**Parameters:**
- `host: str` — Hostname or IP address
- `port: int` — Port number

**Returns:** Socket handle (integer ≥ 1). Throws on failure.

**Example:**
```rhodesia
int: sock = net.socket("example.com", 80)
```

---

### `net.connect(handle)` → `bool`

Connect the socket to its stored host and port.

**Parameters:**
- `handle: int` — Handle returned by `net.socket()`

**Returns:** `true` on success, `false` on failure (host unreachable, refused, etc.).

**Example:**
```rhodesia
bool: ok = net.connect(sock)
if !ok {
    println("Connection failed")
}
```

---

### `net.send(handle, data)` → `int`

Write a string of bytes to a connected socket.

**Parameters:**
- `handle: int` — Connected socket handle
- `data: str` — Bytes to send

**Returns:** Number of bytes actually sent, or `-1` on error.

**Example:**
```rhodesia
int: sent = net.send(sock, "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n")
println("Sent:", sent, "bytes")
```

---

### `net.recv(handle, size)` → `str`

Read up to `size` bytes from a connected socket. May return fewer bytes than requested.

**Parameters:**
- `handle: int` — Connected socket handle
- `size: int` — Maximum number of bytes to read

**Returns:** Received data as a string. Empty string on EOF or error.

**Example:**
```rhodesia
str: chunk = net.recv(sock, 4096)
```

---

### `net.recv_all(handle)` → `str`

Read from a connected socket until the remote side closes the connection (EOF).

**Parameters:**
- `handle: int` — Connected socket handle

**Returns:** All received data concatenated as a string.

**Example:**
```rhodesia
str: response = net.recv_all(sock)
println("Received", string.length(response), "bytes")
```

---

### `net.close(handle)` → `bool`

Close a socket and release its handle. Works for both client and server sockets.

**Parameters:**
- `handle: int` — Any socket handle

**Returns:** `true` if the handle existed and was closed.

**Example:**
```rhodesia
net.close(sock)
```

---

### `net.listen(port [, backlog])` → `int`

Create a server socket bound to `port` on all interfaces (`0.0.0.0`).

**Parameters:**
- `port: int` — Port to bind
- `backlog: int` (optional) — Connection queue length, default `5`

**Returns:** Server socket handle. Throws if the port cannot be bound.

**Example:**
```rhodesia
int: server = net.listen(8080)
println("Listening on :8080")
```

---

### `net.accept(handle)` → `int`

Block until a client connects to the listening socket.

**Parameters:**
- `handle: int` — Server socket handle returned by `net.listen()`

**Returns:** A new connected handle for the accepted client. Throws on error.

**Example:**
```rhodesia
int: client = net.accept(server)
println("Client connected")
```

---

### `net.peer(handle)` → `record`

Return the remote address of a connected socket handle.

**Parameters:**
- `handle: int` — Connected socket handle

**Returns:** `record { host: str, port: int }`

**Example:**
```rhodesia
record: info = net.peer(client)
println("Client IP:", info.host, "Port:", info.port)
```

---

## HTTP API

High-level functions that open a connection, send a request, read the full response, and close the connection automatically.

---

### `net.http_get(url)` → `record`

Perform an HTTP GET request.

**Parameters:**
- `url: str` — Target URL (`http://` only)

**Returns:** [Response record](#response-record). Throws on network errors or invalid URL.

**Example:**
```rhodesia
record: resp = net.http_get("http://httpbin.org/get")
println("Status:", resp.status)
println("Body:", resp.body)
```

---

### `net.http_post(url [, body])` → `record`

Perform an HTTP POST request. `Content-Type` defaults to `application/x-www-form-urlencoded`.

**Parameters:**
- `url: str` — Target URL (`http://` only)
- `body: str` (optional) — Request body

**Returns:** [Response record](#response-record).

**Example:**
```rhodesia
record: resp = net.http_post("http://httpbin.org/post", "name=rhodesia&version=1")
println("Status:", resp.status)
```

---

### `net.http_request(method, url [, body [, headers]])` → `record`

Generic HTTP request for any verb.

**Parameters:**
- `method: str` — HTTP verb (`"GET"`, `"POST"`, `"PUT"`, `"DELETE"`, `"PATCH"`, …)
- `url: str` — Target URL (`http://` only)
- `body: str` (optional) — Request body
- `headers: map` (optional) — Additional request headers as `map{str -> str}`

**Returns:** [Response record](#response-record).

**Example:**
```rhodesia
map: hdrs = {"Content-Type": "application/json", "Authorization": "Bearer token123"}
str: payload = "{\"key\": \"value\"}"
record: resp = net.http_request("PUT", "http://api.example.com/item/1", payload, hdrs)
println("Status:", resp.status)
```

---

## Response Record

All HTTP functions return a `record` with the following fields:

| Field | Type | Description |
|-------|------|-------------|
| `status` | `int` | HTTP status code (200, 404, 500, …) |
| `status_text` | `str` | Status description (`"OK"`, `"Not Found"`, …) |
| `body` | `str` | Response body |
| `headers` | `map` | Response headers (keys are lowercase) |

**Example:**
```rhodesia
record: resp = net.http_get("http://httpbin.org/json")
println(resp.status)             // 200
println(resp.status_text)        // OK
println(resp.body)               // JSON string
str: ct = resp.headers["content-type"]
println(ct)                      // application/json
```

---

## Examples

### Simple HTTP GET

```rhodesia
record: resp = net.http_get("http://httpbin.org/ip")
if resp.status == 200 {
    println("My IP info:", resp.body)
} else {
    println("Error:", resp.status, resp.status_text)
}
```

### POST JSON data

```rhodesia
map: headers = {"Content-Type": "application/json"}
str: body = "{\"user\": \"alice\", \"score\": 42}"
record: resp = net.http_request("POST", "http://httpbin.org/post", body, headers)
println("Response:", resp.body)
```

### Raw TCP connection

```rhodesia
int: sock = net.socket("neverssl.com", 80)
bool: ok = net.connect(sock)

if ok {
    str: req = "GET / HTTP/1.1\r\nHost: neverssl.com\r\nConnection: close\r\n\r\n"
    net.send(sock, req)
    str: raw = net.recv_all(sock)
    println(string.slice(raw, 0, 500))
}

net.close(sock)
```

### Minimal echo server (single connection)

```rhodesia
int: server = net.listen(9000)
println("Waiting for connection on :9000 ...")

int: client = net.accept(server)
record: peer = net.peer(client)
println("Connected:", peer.host, ":", peer.port)

str: msg = net.recv(client, 1024)
println("Received:", msg)

net.send(client, "echo: " + msg)
net.close(client)
net.close(server)
```

---

## Limitations

- **No HTTPS**: TLS is not supported. Use an HTTP-only endpoint or a reverse proxy.
- **IPv4 only**: IPv6 is not implemented.
- **Blocking I/O**: All calls block the interpreter thread. There is no async/non-blocking mode.
- **No timeout**: `net.recv_all()` and `net.accept()` block indefinitely.
