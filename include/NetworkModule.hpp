/**
 * @file NetworkModule.hpp
 * @brief Built-in `net` module: TCP sockets and HTTP/1.1 client.
 *
 * Designed to be #included inside namespace Rhodesia{} in Builtins.hpp,
 * after BuiltinFunc is defined and after all system headers are included.
 *
 * Socket API (net.xxx):
 *   socket(host, port)    -> int   create a TCP socket handle
 *   connect(handle)       -> bool  connect to host:port
 *   send(handle, data)    -> int   bytes sent
 *   recv(handle, size)    -> str   receive up to size bytes
 *   recv_all(handle)      -> str   receive until connection closes
 *   close(handle)         -> bool  close and release handle
 *   listen(port[,backlog])-> int   create a listening server socket
 *   accept(handle)        -> int   block until a client connects
 *   peer(handle)          -> record{host:str, port:int}
 *
 * HTTP API (net.xxx):
 *   http_get(url)                             -> record
 *   http_post(url[, body])                    -> record
 *   http_request(method, url[, body[, hdrs]]) -> record
 *
 * HTTP response record: {status:int, status_text:str, body:str, headers:map}
 *
 * Note: HTTPS is not supported (no TLS dependency).
 */

#ifndef RHODESIA_NETWORK_MODULE_HPP
#define RHODESIA_NETWORK_MODULE_HPP

// All system and standard-library headers are included by Builtins.hpp
// before this file is textually included inside namespace Rhodesia {}.

/**
 * @brief Internal state for a single TCP socket managed by SocketManager.
 */
struct SocketHandle {
    int         fd        = -1;
    std::string host;
    int         port      = 0;
    bool        connected = false;
    bool        listening = false;
    bool        is_open   = false;
};

/**
 * @brief Singleton that owns all open socket file descriptors.
 *
 * Handles are opaque int64_t values returned to Rhodesia code.
 * The manager is responsible for closing the underlying fd on
 * closeSocket() or when the process exits.
 */
class SocketManager {
public:
    static SocketManager& instance() {
        static SocketManager inst;
        return inst;
    }

    /**
     * @brief Allocate a SOCK_STREAM fd and register it under a new handle.
     * @return Handle (>= 1) on success, -1 on failure.
     */
    int64_t createSocket(const std::string& host, int port) {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) return -1;
        int64_t handle = nextHandle_++;
        auto& sh = handles_[handle];
        sh.fd      = fd;
        sh.host    = host;
        sh.port    = port;
        sh.is_open = true;
        return handle;
    }

    /**
     * @brief Connect a previously created socket to its stored host:port.
     * @return true on success.
     */
    bool connectSocket(int64_t handle) {
        auto it = handles_.find(handle);
        if (it == handles_.end() || !it->second.is_open) return false;
        auto& sh = it->second;
        struct addrinfo hints{}, *res = nullptr;
        hints.ai_family   = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        int rc = ::getaddrinfo(sh.host.c_str(), std::to_string(sh.port).c_str(), &hints, &res);
        if (rc != 0 || !res) return false;
        int conn = ::connect(sh.fd, res->ai_addr, res->ai_addrlen);
        ::freeaddrinfo(res);
        sh.connected = (conn == 0);
        return sh.connected;
    }

    /**
     * @brief Write data to a connected socket.
     * @return Number of bytes sent, or -1 on error.
     */
    int64_t sendData(int64_t handle, const std::string& data) {
        auto it = handles_.find(handle);
        if (it == handles_.end() || !it->second.connected) return -1;
        ssize_t sent = ::send(it->second.fd, data.c_str(), data.size(), 0);
        return static_cast<int64_t>(sent);
    }

    /**
     * @brief Read up to @p size bytes from a connected socket.
     * @return Received data, or empty string on EOF / error.
     */
    std::string recvData(int64_t handle, int64_t size) {
        auto it = handles_.find(handle);
        if (it == handles_.end() || !it->second.connected) return "";
        std::string buf(static_cast<size_t>(size), '\0');
        ssize_t n = ::recv(it->second.fd, buf.data(), static_cast<size_t>(size), 0);
        if (n <= 0) return "";
        buf.resize(static_cast<size_t>(n));
        return buf;
    }

    /**
     * @brief Read from a connected socket until the remote side closes.
     * @return All received data concatenated.
     */
    std::string recvAll(int64_t handle) {
        auto it = handles_.find(handle);
        if (it == handles_.end() || !it->second.connected) return "";
        std::string result;
        char tmp[4096];
        ssize_t n;
        while ((n = ::recv(it->second.fd, tmp, sizeof(tmp), 0)) > 0)
            result.append(tmp, static_cast<size_t>(n));
        return result;
    }

    /**
     * @brief Close the fd and remove the handle from the manager.
     * @return true if the handle was found and closed.
     */
    bool closeSocket(int64_t handle) {
        auto it = handles_.find(handle);
        if (it == handles_.end()) return false;
        if (it->second.is_open) close_socket(it->second.fd);
        handles_.erase(it);
        return true;
    }

    /**
     * @brief Create a listening server socket on @p port.
     * @param backlog  Maximum pending connection queue length.
     * @return Handle on success, -1 on failure.
     */
    int64_t listenOn(int port, int backlog = 5) {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) return -1;
        int opt = 1;
        ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                     reinterpret_cast<const char*>(&opt), sizeof(opt));
        struct sockaddr_in addr{};
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port        = htons(static_cast<uint16_t>(port));
        if (::bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0 ||
            ::listen(fd, backlog) < 0) {
            close_socket(fd);
            return -1;
        }
        int64_t handle = nextHandle_++;
        auto& sh = handles_[handle];
        sh.fd        = fd;
        sh.port      = port;
        sh.listening = true;
        sh.is_open   = true;
        return handle;
    }

    /**
     * @brief Block until a client connects to the listening socket.
     * @return A new handle for the accepted client connection, or -1 on error.
     */
    int64_t acceptConnection(int64_t serverHandle) {
        auto it = handles_.find(serverHandle);
        if (it == handles_.end() || !it->second.listening) return -1;
        struct sockaddr_in clientAddr{};
        socklen_t len = sizeof(clientAddr);
        int clientFd = ::accept(it->second.fd,
                                reinterpret_cast<struct sockaddr*>(&clientAddr), &len);
        if (clientFd < 0) return -1;
        int64_t handle = nextHandle_++;
        auto& sh = handles_[handle];
        sh.fd        = clientFd;
        sh.host      = std::string(::inet_ntoa(clientAddr.sin_addr));
        sh.port      = ntohs(clientAddr.sin_port);
        sh.connected = true;
        sh.is_open   = true;
        return handle;
    }

    /**
     * @brief Connect with a wall-clock timeout using non-blocking connect + select.
     * @param timeout_secs  Maximum seconds to wait. 0 = blocking (same as connectSocket).
     * @return true on success, false on timeout or error.
     */
    bool connectSocketTimeout(int64_t handle, int timeout_secs) {
        auto it = handles_.find(handle);
        if (it == handles_.end() || !it->second.is_open) return false;
        auto& sh = it->second;

        struct addrinfo hints{}, *res = nullptr;
        hints.ai_family   = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        int rc = ::getaddrinfo(sh.host.c_str(), std::to_string(sh.port).c_str(), &hints, &res);
        if (rc != 0 || !res) return false;

        // Switch to non-blocking
        socket_set_nonblocking(sh.fd);

        int conn = ::connect(sh.fd, res->ai_addr, res->ai_addrlen);
        ::freeaddrinfo(res);

        if (conn == 0) {
            socket_set_blocking(sh.fd);
            sh.connected = true;
            return true;
        }

        if (LAST_SOCKET_ERROR() != EINPROGRESS) {
            socket_set_blocking(sh.fd);
            return false;
        }

        // Wait for writability (connection complete or error)
        fd_set wfds, efds;
        FD_ZERO(&wfds); FD_SET(sh.fd, &wfds);
        FD_ZERO(&efds); FD_SET(sh.fd, &efds);
        struct timeval tv{ timeout_secs, 0 };

        int sel = ::select(sh.fd + 1, nullptr, &wfds, &efds, &tv);
        socket_set_blocking(sh.fd); // Restore blocking

        if (sel <= 0) return false; // Timeout (0) or select error (-1)

        int err = 0;
        socklen_t len = sizeof(err);
        ::getsockopt(sh.fd, SOL_SOCKET, SO_ERROR,
                     reinterpret_cast<char*>(&err), &len);
        sh.connected = (err == 0);
        return sh.connected;
    }

    /**
     * @brief Set SO_RCVTIMEO and SO_SNDTIMEO on the socket.
     * @param secs  Timeout in seconds. 0 = disable timeout (blocking).
     */
    bool setSocketTimeout(int64_t handle, int secs) {
        auto it = handles_.find(handle);
        if (it == handles_.end()) return false;
        struct timeval tv{ secs, 0 };
        ::setsockopt(it->second.fd, SOL_SOCKET, SO_RCVTIMEO,
                     reinterpret_cast<const char*>(&tv), sizeof(tv));
        ::setsockopt(it->second.fd, SOL_SOCKET, SO_SNDTIMEO,
                     reinterpret_cast<const char*>(&tv), sizeof(tv));
        return true;
    }

    /** @return Pointer to the SocketHandle for the given handle, or nullptr. */
    SocketHandle* getHandle(int64_t handle) {
        auto it = handles_.find(handle);
        return it == handles_.end() ? nullptr : &it->second;
    }

private:
    std::unordered_map<int64_t, SocketHandle> handles_;
    int64_t nextHandle_ = 1;
    SocketManager() = default;
};

/**
 * @brief Parsed components of an HTTP URL.
 */
struct ParsedUrl {
    std::string scheme;  ///< "http" (https not supported)
    std::string host;
    int         port = 80;
    std::string path;    ///< includes query string, always starts with '/'
};

/**
 * @brief Decompose a URL string into its scheme, host, port and path.
 *
 * Supports http:// prefix and host:port notation.
 * Missing path defaults to "/".
 */
inline ParsedUrl parseUrl(const std::string& url) {
    ParsedUrl result;
    std::string rest = url;

    auto schemeEnd = rest.find("://");
    if (schemeEnd != std::string::npos) {
        result.scheme = rest.substr(0, schemeEnd);
        rest = rest.substr(schemeEnd + 3);
    } else {
        result.scheme = "http";
    }
    result.port = (result.scheme == "https") ? 443 : 80;

    auto pathStart = rest.find('/');
    std::string hostPart;
    if (pathStart != std::string::npos) {
        hostPart = rest.substr(0, pathStart);
        rest     = rest.substr(pathStart);
    } else {
        hostPart = rest;
        rest     = "/";
    }

    auto portSep = hostPart.find(':');
    if (portSep != std::string::npos) {
        result.host = hostPart.substr(0, portSep);
        result.port = std::stoi(hostPart.substr(portSep + 1));
    } else {
        result.host = hostPart;
    }

    result.path = rest.empty() ? "/" : rest;
    return result;
}

/**
 * @brief Parsed HTTP response: status, headers (lowercase keys) and body.
 */
struct HttpResponse {
    int                                status_code = 0;
    std::string                        status_text;
    std::map<std::string, std::string> headers;  ///< keys are lowercased
    std::string                        body;
};

/**
 * @brief Perform a raw HTTP/1.1 request and return the parsed response.
 *
 * Opens a TCP connection, sends the request, reads the full response, and
 * handles chunked transfer encoding.  Throws std::runtime_error on network
 * or protocol errors.
 *
 * @param method        HTTP verb (GET, POST, PUT, …)
 * @param url           Target URL (http:// only)
 * @param body          Optional request body
 * @param extra_headers Additional headers to include in the request
 */
inline HttpResponse performHttpRequest(
    const std::string& method,
    const std::string& url,
    const std::string& body = "",
    const std::map<std::string, std::string>& extra_headers = {})
{
    auto parsed = parseUrl(url);
    if (parsed.scheme == "https")
        throw std::runtime_error("HTTPS not supported (no TLS). Use HTTP.");

    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int rc = ::getaddrinfo(parsed.host.c_str(),
                           std::to_string(parsed.port).c_str(), &hints, &res);
    if (rc != 0 || !res)
        throw std::runtime_error("Could not resolve host: " + parsed.host);

    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { ::freeaddrinfo(res); throw std::runtime_error("socket() failed"); }

    if (::connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
        ::freeaddrinfo(res);
        close_socket(fd);
        throw std::runtime_error("connect() failed: " + parsed.host
                                 + ":" + std::to_string(parsed.port));
    }
    ::freeaddrinfo(res);

    std::ostringstream req;
    req << method << " " << parsed.path << " HTTP/1.1\r\n"
        << "Host: " << parsed.host << "\r\n"
        << "Connection: close\r\n"
        << "User-Agent: Rhodesia/0.1\r\n";
    for (const auto& [k, v] : extra_headers)
        req << k << ": " << v << "\r\n";
    if (!body.empty()) {
        req << "Content-Length: " << body.size() << "\r\n";
        if (extra_headers.find("Content-Type") == extra_headers.end())
            req << "Content-Type: application/octet-stream\r\n";
    }
    req << "\r\n";
    if (!body.empty()) req << body;

    std::string reqStr = req.str();
    ::send(fd, reqStr.c_str(), reqStr.size(), 0);

    std::string raw;
    char tmp[4096];
    ssize_t n;
    while ((n = ::recv(fd, tmp, sizeof(tmp), 0)) > 0)
        raw.append(tmp, static_cast<size_t>(n));
    close_socket(fd);

    HttpResponse resp;
    auto headerEnd = raw.find("\r\n\r\n");
    if (headerEnd == std::string::npos) { resp.body = raw; return resp; }

    std::string headerSection = raw.substr(0, headerEnd);
    resp.body = raw.substr(headerEnd + 4);

    std::istringstream hstream(headerSection);
    std::string statusLine;
    std::getline(hstream, statusLine);
    if (!statusLine.empty() && statusLine.back() == '\r') statusLine.pop_back();

    auto sp1 = statusLine.find(' ');
    if (sp1 != std::string::npos) {
        auto sp2 = statusLine.find(' ', sp1 + 1);
        if (sp2 != std::string::npos) {
            try { resp.status_code = std::stoi(statusLine.substr(sp1 + 1, sp2 - sp1 - 1)); }
            catch (...) {}
            resp.status_text = statusLine.substr(sp2 + 1);
        }
    }

    std::string line;
    while (std::getline(hstream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        auto sep = line.find(':');
        if (sep == std::string::npos) continue;
        std::string key = line.substr(0, sep);
        for (char& c : key) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        std::string val = line.substr(sep + 1);
        auto trim = val.find_first_not_of(' ');
        if (trim != std::string::npos) val = val.substr(trim);
        resp.headers[key] = val;
    }

    // Decode chunked transfer encoding if present
    auto cit = resp.headers.find("transfer-encoding");
    if (cit != resp.headers.end() && cit->second.find("chunked") != std::string::npos) {
        std::string decoded;
        std::istringstream bs(resp.body);
        while (bs) {
            std::string sizeLine;
            std::getline(bs, sizeLine);
            if (!sizeLine.empty() && sizeLine.back() == '\r') sizeLine.pop_back();
            if (sizeLine.empty()) continue;
            size_t chunkSize = 0;
            try { chunkSize = std::stoul(sizeLine, nullptr, 16); } catch (...) { break; }
            if (chunkSize == 0) break;
            std::string chunk(chunkSize, '\0');
            bs.read(chunk.data(), static_cast<std::streamsize>(chunkSize));
            decoded += chunk;
            bs.ignore(2);
        }
        resp.body = decoded;
    }

    return resp;
}

/**
 * @brief Register all `net` module functions and constants into Builtins.
 *
 * Called once from Builtins::registerAll().
 *
 * @param modules_         Module function map owned by Builtins.
 * @param moduleConstants_ Module constant map owned by Builtins.
 */
inline void registerNetworkModule(
    std::unordered_map<std::string,
        std::unordered_map<std::string, BuiltinFunc>>& modules_,
    std::unordered_map<std::string,
        std::unordered_map<std::string, RhoValue>>& moduleConstants_)
{
    auto& net      = modules_["net"];
    auto& netConst = moduleConstants_["net"];

    netConst["DEFAULT_PORT_HTTP"] = int64_t(80);
    netConst["DEFAULT_TIMEOUT"]   = int64_t(30);

    /**
     * net.socket(host: str, port: int) -> int
     * Create a TCP socket for the given host and port.
     * The socket is NOT connected yet; call net.connect() next.
     * Returns an opaque handle used by all other net functions.
     */
    net["socket"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
        if (args.size() != 2)
            throw ArgumentError::wrongCount("net.socket", 2, args.size(), loc);
        const auto* host = std::get_if<std::string>(&args[0]);
        if (!host) throw TypeError("net.socket: host must be a string", loc);
        int64_t h = SocketManager::instance().createSocket(*host, static_cast<int>(toInt(args[1])));
        if (h < 0) throw RuntimeError("net.socket: failed to create socket", loc);
        return h;
    };

    /**
     * net.connect(handle: int [, timeout: int]) -> bool
     * Connect the socket identified by handle to its stored host:port.
     * If timeout (seconds) is provided and > 0, the call fails after that many
     * seconds instead of blocking indefinitely.
     * Returns true on success, false on failure or timeout.
     */
    net["connect"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
        if (args.empty() || args.size() > 2)
            throw ArgumentError::wrongCount("net.connect", 1, args.size(), loc);
        int64_t handle = toInt(args[0]);
        if (args.size() == 2) {
            int secs = static_cast<int>(toInt(args[1]));
            return SocketManager::instance().connectSocketTimeout(handle, secs);
        }
        return SocketManager::instance().connectSocket(handle);
    };

    /**
     * net.send(handle: int, data: str) -> int
     * Write data to a connected socket.
     * Returns the number of bytes actually sent, or -1 on error.
     */
    net["send"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
        if (args.size() != 2)
            throw ArgumentError::wrongCount("net.send", 2, args.size(), loc);
        const auto* data = std::get_if<std::string>(&args[1]);
        if (!data) throw TypeError("net.send: data must be a string", loc);
        return SocketManager::instance().sendData(toInt(args[0]), *data);
    };

    /**
     * net.recv(handle: int, size: int) -> str
     * Read up to size bytes from a connected socket.
     * Returns the received data, or an empty string on EOF or error.
     */
    net["recv"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
        if (args.size() != 2)
            throw ArgumentError::wrongCount("net.recv", 2, args.size(), loc);
        return SocketManager::instance().recvData(toInt(args[0]), toInt(args[1]));
    };

    /**
     * net.recv_all(handle: int) -> str
     * Read all data until the remote side closes the connection.
     * Blocks until EOF.
     */
    net["recv_all"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
        if (args.size() != 1)
            throw ArgumentError::wrongCount("net.recv_all", 1, args.size(), loc);
        return SocketManager::instance().recvAll(toInt(args[0]));
    };

    /**
     * net.close(handle: int) -> bool
     * Close the socket and release its handle.
     * Returns true if the handle was valid and has been closed.
     */
    net["close"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
        if (args.size() != 1)
            throw ArgumentError::wrongCount("net.close", 1, args.size(), loc);
        return SocketManager::instance().closeSocket(toInt(args[0]));
    };

    /**
     * net.listen(port: int [, backlog: int]) -> int
     * Bind and listen on the given port.  backlog defaults to 5.
     * Returns a server socket handle for use with net.accept().
     * Throws if the port cannot be bound.
     */
    net["listen"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
        if (args.empty() || args.size() > 2)
            throw ArgumentError::wrongCount("net.listen", 1, args.size(), loc);
        int port    = static_cast<int>(toInt(args[0]));
        int backlog = args.size() == 2 ? static_cast<int>(toInt(args[1])) : 5;
        int64_t h = SocketManager::instance().listenOn(port, backlog);
        if (h < 0)
            throw RuntimeError("net.listen: could not bind port " + std::to_string(port), loc);
        return h;
    };

    /**
     * net.accept(handle: int) -> int
     * Block until a client connects to the listening socket.
     * Returns a new connected handle for the client.
     * Throws on error.
     */
    net["accept"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
        if (args.size() != 1)
            throw ArgumentError::wrongCount("net.accept", 1, args.size(), loc);
        int64_t h = SocketManager::instance().acceptConnection(toInt(args[0]));
        if (h < 0) throw RuntimeError("net.accept: failed to accept connection", loc);
        return h;
    };

    /**
     * net.peer(handle: int) -> record{host: str, port: int}
     * Return the remote address associated with a socket handle.
     * Useful after net.accept() to identify the connected client.
     */
    net["peer"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
        if (args.size() != 1)
            throw ArgumentError::wrongCount("net.peer", 1, args.size(), loc);
        SocketHandle* sh = SocketManager::instance().getHandle(toInt(args[0]));
        if (!sh) throw RuntimeError("net.peer: invalid socket handle", loc);
        auto rec = std::make_shared<RhoRecord>();
        rec->setField("host", sh->host);
        rec->setField("port", int64_t(sh->port));
        return rec;
    };

    /**
     * net.set_timeout(handle: int, seconds: int) -> bool
     * Set SO_RCVTIMEO and SO_SNDTIMEO on the socket so that recv/send
     * operations fail with an empty result after the given number of seconds.
     * Pass 0 to disable the timeout (blocking mode).
     */
    net["set_timeout"] = [](const std::vector<RhoValue>& args, SourceLocation loc) -> RhoValue {
        if (args.size() != 2)
            throw ArgumentError::wrongCount("net.set_timeout", 2, args.size(), loc);
        return SocketManager::instance().setSocketTimeout(
            toInt(args[0]), static_cast<int>(toInt(args[1])));
    };

    // Shared helper: convert an HttpResponse into a RhoRecord.
    auto makeRespRecord = [](const HttpResponse& resp) -> RhoValue {
        auto rec = std::make_shared<RhoRecord>();
        rec->setField("status",      int64_t(resp.status_code));
        rec->setField("status_text", resp.status_text);
        rec->setField("body",        resp.body);
        auto hmap = std::make_shared<RhoMap>();
        for (const auto& [k, v] : resp.headers) hmap->set(k, v);
        rec->setField("headers", RhoValue(hmap));
        return rec;
    };

    /**
     * net.http_get(url: str) -> record
     * Perform an HTTP GET request and return the parsed response.
     * Response record fields: status, status_text, body, headers.
     * Throws on network errors or if the URL uses HTTPS.
     */
    net["http_get"] = [makeRespRecord](const std::vector<RhoValue>& args,
                                       SourceLocation loc) -> RhoValue {
        if (args.size() != 1)
            throw ArgumentError::wrongCount("net.http_get", 1, args.size(), loc);
        const auto* url = std::get_if<std::string>(&args[0]);
        if (!url) throw TypeError("net.http_get: url must be a string", loc);
        try {
            return makeRespRecord(performHttpRequest("GET", *url));
        } catch (const std::exception& e) {
            throw RuntimeError(std::string("net.http_get: ") + e.what(), loc);
        }
    };

    /**
     * net.http_post(url: str [, body: str]) -> record
     * Perform an HTTP POST request.
     * Content-Type defaults to application/x-www-form-urlencoded.
     * Response record fields: status, status_text, body, headers.
     */
    net["http_post"] = [makeRespRecord](const std::vector<RhoValue>& args,
                                        SourceLocation loc) -> RhoValue {
        if (args.size() < 1 || args.size() > 2)
            throw ArgumentError::wrongCount("net.http_post", 2, args.size(), loc);
        const auto* url = std::get_if<std::string>(&args[0]);
        if (!url) throw TypeError("net.http_post: url must be a string", loc);
        std::string body;
        if (args.size() == 2) {
            const auto* b = std::get_if<std::string>(&args[1]);
            if (!b) throw TypeError("net.http_post: body must be a string", loc);
            body = *b;
        }
        try {
            return makeRespRecord(performHttpRequest("POST", *url, body,
                {{"Content-Type", "application/x-www-form-urlencoded"}}));
        } catch (const std::exception& e) {
            throw RuntimeError(std::string("net.http_post: ") + e.what(), loc);
        }
    };

    /**
     * net.http_request(method: str, url: str [, body: str [, headers: map]]) -> record
     * Generic HTTP request for any verb (GET, POST, PUT, DELETE, PATCH, ...).
     * headers must be a map{str -> str} of additional request headers.
     * Response record fields: status, status_text, body, headers.
     */
    net["http_request"] = [makeRespRecord](const std::vector<RhoValue>& args,
                                            SourceLocation loc) -> RhoValue {
        if (args.size() < 2 || args.size() > 4)
            throw ArgumentError::wrongCount("net.http_request", 2, args.size(), loc);
        const auto* method = std::get_if<std::string>(&args[0]);
        const auto* url    = std::get_if<std::string>(&args[1]);
        if (!method) throw TypeError("net.http_request: method must be a string", loc);
        if (!url)    throw TypeError("net.http_request: url must be a string", loc);

        std::string body;
        if (args.size() >= 3) {
            const auto* b = std::get_if<std::string>(&args[2]);
            if (!b) throw TypeError("net.http_request: body must be a string", loc);
            body = *b;
        }

        std::map<std::string, std::string> extra;
        if (args.size() == 4) {
            const auto* hmap = std::get_if<std::shared_ptr<RhoMap>>(&args[3]);
            if (!hmap) throw TypeError("net.http_request: headers must be a map", loc);
            for (auto it = (*hmap)->begin(); it != (*hmap)->end(); ++it) {
                if (const auto* vs = std::get_if<std::string>(&it->second))
                    extra[it->first] = *vs;
            }
        }

        try {
            return makeRespRecord(performHttpRequest(*method, *url, body, extra));
        } catch (const std::exception& e) {
            throw RuntimeError(std::string("net.http_request: ") + e.what(), loc);
        }
    };
}

#endif // RHODESIA_NETWORK_MODULE_HPP
