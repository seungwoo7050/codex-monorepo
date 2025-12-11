/**
 * [모듈] webserv-cpp17/src/main.cpp
 * 설명:
 *   - select 기반 이벤트 루프로 여러 클라이언트 연결을 동시에 처리한다.
 *   - 연결 타임아웃을 감시하고 최대 요청 수를 처리하면 종료한다.
 * 버전: v0.2.0
 * 관련 설계문서:
 *   - design/webserv-cpp17/v0.1.0-basic-http-server.md
 *   - design/webserv-cpp17/v0.2.0-multi-connection-loop.md
 * 변경 이력:
 *   - v0.1.0: 단일 연결 처리 및 고정 응답 송신 기능 추가
 *   - v0.2.0: 비동기 다중 연결 루프와 타임아웃 관리 추가
 * 테스트:
 *   - tests/test_webserv.sh
 *   - tests/test_webserv_multi.sh
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include <chrono>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace {

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

struct Connection {
    int fd;
    std::string buffer;
    std::chrono::steady_clock::time_point last_active;
};

/**
 * buildResponse
 * 설명:
 *   - v0.2.0 버전에 맞춘 고정 HTTP 응답을 구성한다.
 * 입력:
 *   - 없음
 * 출력:
 *   - 클라이언트로 전송할 HTTP 문자열
 * 에러:
 *   - 없음 (고정 문자열 생성)
 * 관련 설계문서:
 *   - design/webserv-cpp17/v0.2.0-multi-connection-loop.md
 * 관련 테스트:
 *   - tests/test_webserv_multi.sh
 */
std::string buildResponse() {
    const std::string body = "Hello from webserv v0.2.0\n";
    std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "Connection: close\r\n\r\n" + body;

    return response;
}

/**
 * closeConnection
 * 설명:
 *   - 연결 FD를 닫고 목록에서 제거한다.
 * 입력:
 *   - connections: 관리 중인 연결 목록
 *   - index: 제거할 연결 인덱스
 * 출력:
 *   - 없음
 * 에러:
 *   - close 실패 시 로그만 남기며 계속 진행한다.
 * 관련 설계문서:
 *   - design/webserv-cpp17/v0.2.0-multi-connection-loop.md
 */
void closeConnection(std::vector<Connection> &connections, std::size_t index) {
    if (index >= connections.size()) {
        return;
    }
    ::close(connections[index].fd);
    connections.erase(connections.begin() + index);
}

/**
 * handleConnections
 * 설명:
 *   - select 결과에 따라 새 연결을 수락하고 기존 연결에서 요청을 읽어 응답한다.
 * 입력:
 *   - listen_fd: 리슨 소켓
 *   - connections: 관리 중인 연결 벡터
 *   - timeout: 연결 타임아웃
 *   - max_requests: 최대 처리 요청 수
 *   - handled: 지금까지 처리 완료한 연결 개수
 * 출력:
 *   - 요청 처리 중 오류가 없으면 true
 * 에러:
 *   - 수락/송수신 오류는 stderr에 기록하고 해당 연결만 종료한다.
 * 관련 설계문서:
 *   - design/webserv-cpp17/v0.2.0-multi-connection-loop.md
 * 관련 테스트:
 *   - tests/test_webserv_multi.sh
 */
bool handleConnections(int listen_fd,
                      std::vector<Connection> &connections,
                      std::chrono::milliseconds timeout,
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

            connections.push_back(Connection{client_fd, std::string(), now});
        }
    }

    for (std::size_t i = 0; i < connections.size();) {
        bool erased = false;
        if (FD_ISSET(connections[i].fd, &read_set)) {
            char buffer[2048];
            ssize_t received = recv(connections[i].fd, buffer, sizeof(buffer), 0);
            if (received <= 0) {
                closeConnection(connections, i);
                erased = true;
            } else {
                connections[i].buffer.append(buffer, static_cast<std::size_t>(received));
                connections[i].last_active = now;
                std::string response = buildResponse();
                ssize_t sent = send(connections[i].fd, response.c_str(), response.size(), 0);
                if (sent < 0) {
                    std::cerr << "응답 송신 실패: " << std::strerror(errno) << std::endl;
                }
                closeConnection(connections, i);
                erased = true;
                ++handled;
                if (handled >= max_requests) {
                    break;
                }
            }
        }

        if (!erased) {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - connections[i].last_active) > timeout) {
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
