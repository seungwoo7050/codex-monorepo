/**
 * [모듈] webserv-cpp17/src/main.cpp
 * 설명:
 *   - HTTP/1.1 Host 헤더와 keep-alive를 지원하는 단일 스레드 이벤트 루프를 제공한다.
 *   - select 기반으로 다중 연결을 처리하며 요청 파싱과 응답 전송을 순차적으로 수행한다.
 * 버전: v0.3.0
 * 관련 설계문서:
 *   - design/webserv-cpp17/v0.1.0-basic-http-server.md
 *   - design/webserv-cpp17/v0.2.0-multi-connection-loop.md
 *   - design/webserv-cpp17/v0.3.0-http11-core.md
 * 변경 이력:
 *   - v0.1.0: 단일 연결 처리 및 고정 응답 송신 기능 추가
 *   - v0.2.0: 비동기 다중 연결 루프와 타임아웃 관리 추가
 *   - v0.3.0: Host 헤더 검증, keep-alive 처리, 요청 파싱 개선
 * 테스트:
 *   - tests/test_webserv.sh
 *   - tests/test_webserv_multi.sh
 *   - tests/test_webserv_host_header.sh
 *   - tests/test_webserv_keepalive.sh
 */

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>

namespace {

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
};

struct Connection {
    int fd;
    std::string buffer;
    std::chrono::steady_clock::time_point last_active;
    bool should_close;
};

/**
 * createListenSocket
 * 설명:
 *   - IPv4 TCP 소켓을 생성하고 지정된 포트로 바인드한 뒤 리슨 상태로 전환한다.
 * 입력:
 *   - port: 수신 포트 번호
 * 출력:
 *   - 성공 시 수신 소켓 FD, 실패 시 -1
 * 에러:
 *   - 소켓 생성/바인드/리슨 과정 실패 시 stderr에 한국어 메시지 출력 후 -1 반환
 * 관련 설계문서:
 *   - design/webserv-cpp17/v0.2.0-multi-connection-loop.md
 * 관련 테스트:
 *   - tests/test_webserv_multi.sh
 */
int createListenSocket(uint16_t port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        std::cerr << "소켓 생성 실패: " << std::strerror(errno) << std::endl;
        return -1;
    }

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "SO_REUSEADDR 설정 실패: " << std::strerror(errno) << std::endl;
        ::close(fd);
        return -1;
    }

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        std::cerr << "포트 바인드 실패: " << std::strerror(errno) << std::endl;
        ::close(fd);
        return -1;
    }

    if (listen(fd, 16) < 0) {
        std::cerr << "리슨 실패: " << std::strerror(errno) << std::endl;
        ::close(fd);
        return -1;
    }

    int flags = fcntl(fd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

    return fd;
}

/**
 * toLower
 * 설명:
 *   - 헤더 키 비교를 단순화하기 위해 소문자로 변환한다.
 */
std::string toLower(const std::string &input) {
    std::string lowered;
    lowered.reserve(input.size());
    for (char c : input) {
        lowered.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return lowered;
}

/**
 * parseHttpRequest
 * 설명:
 *   - 수신 버퍼에서 HTTP 요청 라인과 헤더를 파싱하고 소모한 길이를 반환한다.
 * 입력:
 *   - buffer: 현재까지 누적된 원시 요청 문자열
 * 출력:
 *   - 성공 시 HttpRequest와 소모 길이, 실패 시 false
 * 에러:
 *   - 헤더 구분자가 없으면 false를 반환하여 추가 데이터를 기다린다.
 *   - 형식 오류 시 false를 반환하고 호출자가 연결을 종료하도록 한다.
 * 관련 설계문서:
 *   - design/webserv-cpp17/v0.3.0-http11-core.md
 * 관련 테스트:
 *   - tests/test_webserv_host_header.sh
 */
bool parseHttpRequest(const std::string &buffer, HttpRequest &out, std::size_t &consumed) {
    std::size_t header_end = buffer.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return false;
    }

    std::istringstream stream(buffer.substr(0, header_end));
    std::string request_line;
    if (!std::getline(stream, request_line)) {
        consumed = header_end + 4;
        return false;
    }

    if (!request_line.empty() && request_line.back() == '\r') {
        request_line.pop_back();
    }

    std::istringstream line_stream(request_line);
    if (!(line_stream >> out.method >> out.path >> out.version)) {
        consumed = header_end + 4;
        return false;
    }

    std::string header_line;
    while (std::getline(stream, header_line)) {
        if (!header_line.empty() && header_line.back() == '\r') {
            header_line.pop_back();
        }
        if (header_line.empty()) {
            continue;
        }
        std::size_t colon = header_line.find(':');
        if (colon == std::string::npos) {
            continue;
        }
        std::string key = header_line.substr(0, colon);
        std::string value = header_line.substr(colon + 1);
        while (!value.empty() && value.front() == ' ') {
            value.erase(value.begin());
        }
        out.headers[toLower(key)] = value;
    }

    consumed = header_end + 4;
    return true;
}

/**
 * buildResponse
 * 설명:
 *   - 상태 코드, 본문, keep-alive 여부에 맞춰 HTTP 응답을 구성한다.
 * 입력:
 *   - status: HTTP 상태 코드
 *   - message: 본문 문자열
 *   - keep_alive: 연결을 유지할지 여부
 * 출력:
 *   - 직렬화된 HTTP 응답 문자열
 * 에러:
 *   - 없음 (고정 문자열 조합)
 * 관련 설계문서:
 *   - design/webserv-cpp17/v0.3.0-http11-core.md
 * 관련 테스트:
 *   - tests/test_webserv_keepalive.sh
 */
std::string buildResponse(int status, const std::string &message, bool keep_alive) {
    std::string status_line;
    if (status == 200) {
        status_line = "HTTP/1.1 200 OK\r\n";
    } else {
        status_line = "HTTP/1.1 400 Bad Request\r\n";
    }

    std::string connection_header = keep_alive ? "keep-alive" : "close";
    std::string body = message;
    std::string response = status_line +
        "Content-Type: text/plain; charset=utf-8\r\n" +
        "Content-Length: " + std::to_string(body.size()) + "\r\n" +
        "Connection: " + connection_header + "\r\n\r\n" + body;

    return response;
}

/**
 * closeConnection
 * 설명:
 *   - 연결 FD를 닫고 목록에서 제거한다.
 */
void closeConnection(std::vector<Connection> &connections, std::size_t index) {
    if (index >= connections.size()) {
        return;
    }

    ::close(connections[index].fd);
    connections.erase(connections.begin() + static_cast<std::ptrdiff_t>(index));
}

/**
 * handleConnections
 * 설명:
 *   - select를 사용해 다중 연결을 감시하고, 요청 파싱/응답 전송을 수행한다.
 * 입력:
 *   - listen_fd: 수신 소켓 FD
 *   - connections: 현재 열린 연결 목록
 *   - timeout: 비활성 타임아웃
 *   - max_requests: 처리할 최대 요청 수
 *   - handled: 이미 처리한 요청 수
 * 출력:
 *   - 에러 없이 루프를 유지하면 true, 치명적 오류 시 false
 * 에러:
 *   - select/recv/send 실패 시 stderr에 한국어 메시지를 남기고 false를 반환한다.
 * 관련 설계문서:
 *   - design/webserv-cpp17/v0.3.0-http11-core.md
 * 관련 테스트:
 *   - tests/test_webserv_keepalive.sh
 */
bool handleConnections(
    int listen_fd,
    std::vector<Connection> &connections,
    const std::chrono::milliseconds &timeout,
    std::size_t max_requests,
    std::size_t &handled) {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(listen_fd, &read_set);

    int max_fd = listen_fd;
    for (const Connection &conn : connections) {
        FD_SET(conn.fd, &read_set);
        if (conn.fd > max_fd) {
            max_fd = conn.fd;
        }
    }

    timeval tv;
    tv.tv_sec = timeout.count() / 1000;
    tv.tv_usec = (timeout.count() % 1000) * 1000;

    int ready = select(max_fd + 1, &read_set, nullptr, nullptr, &tv);
    auto now = std::chrono::steady_clock::now();
    if (ready < 0) {
        if (errno == EINTR) {
            return true;
        }
        std::cerr << "select 호출 실패: " << std::strerror(errno) << std::endl;
        return false;
    }

    if (FD_ISSET(listen_fd, &read_set)) {
        while (true) {
            sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(listen_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_len);
            if (client_fd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }
                std::cerr << "연결 수락 실패: " << std::strerror(errno) << std::endl;
                break;
            }

            int flags = fcntl(client_fd, F_GETFL, 0);
            if (flags >= 0) {
                fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
            }

            connections.push_back(Connection{client_fd, std::string(), now, false});
        }
    }

    for (std::size_t i = 0; i < connections.size();) {
        bool erased = false;
        Connection &conn = connections[i];
        if (FD_ISSET(conn.fd, &read_set)) {
            char buffer[4096];
            ssize_t received = recv(conn.fd, buffer, sizeof(buffer), 0);
            if (received <= 0) {
                closeConnection(connections, i);
                erased = true;
            } else {
                conn.buffer.append(buffer, static_cast<std::size_t>(received));
                conn.last_active = now;

                while (true) {
                    HttpRequest request;
                    std::size_t consumed = 0;
                    if (!parseHttpRequest(conn.buffer, request, consumed)) {
                        break;
                    }

                    bool is_http11 = (request.version == "HTTP/1.1");
                    bool has_host = request.headers.find("host") != request.headers.end();

                    bool keep_alive = false;
                    if (is_http11) {
                        keep_alive = request.headers.find("connection") == request.headers.end() ||
                                     toLower(request.headers.find("connection")->second) != "close";
                    } else if (request.version == "HTTP/1.0") {
                        auto it = request.headers.find("connection");
                        if (it != request.headers.end() && toLower(it->second) == "keep-alive") {
                            keep_alive = true;
                        }
                    }

                    std::string body;
                    int status = 200;
                    if (is_http11 && !has_host) {
                        status = 400;
                        keep_alive = false;
                        body = "Missing Host header\n";
                    } else {
                        std::string host_value = has_host ? request.headers["host"] : "host-not-set";
                        body = "Hello from webserv v0.3.0\nHost: " + host_value + "\n";
                        body += keep_alive ? "Connection: keep-alive\n" : "Connection: close\n";
                    }

                    std::string response = buildResponse(status, body, keep_alive);
                    ssize_t sent = send(conn.fd, response.c_str(), response.size(), 0);
                    if (sent < 0) {
                        std::cerr << "응답 송신 실패: " << std::strerror(errno) << std::endl;
                        closeConnection(connections, i);
                        erased = true;
                        break;
                    }

                    handled++;
                    conn.buffer.erase(0, consumed);
                    if (!keep_alive || handled >= max_requests) {
                        closeConnection(connections, i);
                        erased = true;
                        break;
                    }
                }
            }
        }

        if (!erased) {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - conn.last_active) > timeout) {
                std::cerr << "연결 타임아웃 발생" << std::endl;
                closeConnection(connections, i);
                erased = true;
                ++handled;
                if (handled >= max_requests) {
                    break;
                }
            }
        }

        if (!erased) {
            ++i;
        }
    }

    return true;
}

}  // namespace

int main(int argc, char *argv[]) {
    uint16_t port = 8080;
    if (argc >= 2) {
        port = static_cast<uint16_t>(std::atoi(argv[1]));
    }

    std::size_t max_requests = 3;
    if (argc >= 3) {
        max_requests = static_cast<std::size_t>(std::strtoul(argv[2], nullptr, 10));
        if (max_requests == 0) {
            max_requests = 1;
        }
    }

    int listen_fd = createListenSocket(port);
    if (listen_fd < 0) {
        return EXIT_FAILURE;
    }

    std::vector<Connection> connections;
    std::size_t handled = 0;
    const std::chrono::milliseconds timeout(1500);
    const auto start_time = std::chrono::steady_clock::now();

    bool ok = true;
    while (handled < max_requests) {
        ok = handleConnections(listen_fd, connections, timeout, max_requests, handled);
        if (!ok) {
            break;
        }

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count() > 10) {
            std::cerr << "최대 런타임을 초과하여 루프를 종료합니다." << std::endl;
            break;
        }
    }

    for (Connection &conn : connections) {
        ::close(conn.fd);
    }
    ::close(listen_fd);

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
