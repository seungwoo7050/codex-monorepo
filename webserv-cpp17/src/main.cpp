/**
 * [모듈] webserv-cpp17/src/main.cpp
 * 설명:
 *   - 단일 HTTP GET 요청을 처리하는 기본 서버를 실행한다.
 *   - 첫 번째 연결을 처리한 후 서버를 종료하여 초기 동작을 검증한다.
 * 버전: v0.1.0
 * 관련 설계문서:
 *   - design/webserv-cpp17/v0.1.0-basic-http-server.md
 * 변경 이력:
 *   - v0.1.0: 단일 연결 처리 및 고정 응답 송신 기능 추가
 * 테스트:
 *   - tests/test_webserv.sh
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

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
 *   - design/webserv-cpp17/v0.1.0-basic-http.md
 * 관련 테스트:
 *   - tests/test_webserv.sh
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

    if (listen(fd, 1) < 0) {
        std::cerr << "리슨 실패: " << std::strerror(errno) << std::endl;
        ::close(fd);
        return -1;
    }

    return fd;
}

/**
 * respondOnce
 * 설명:
 *   - 첫 번째 연결을 수락하고 요청 헤더를 읽은 뒤 고정된 200 응답을 전송한다.
 * 입력:
 *   - listen_fd: 수신 소켓
 * 출력:
 *   - 성공 시 true, 실패 시 false
 * 에러:
 *   - accept/read/write 실패 시 stderr에 오류 메시지를 남기고 false 반환
 * 관련 설계문서:
 *   - design/webserv-cpp17/v0.1.0-basic-http.md
 * 관련 테스트:
 *   - tests/test_webserv.sh
 */
bool respondOnce(int listen_fd) {
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(listen_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_len);
    if (client_fd < 0) {
        std::cerr << "연결 수락 실패: " << std::strerror(errno) << std::endl;
        return false;
    }

    char buffer[1024];
    ssize_t received = recv(client_fd, buffer, sizeof(buffer), 0);
    if (received < 0) {
        std::cerr << "요청 수신 실패: " << std::strerror(errno) << std::endl;
        ::close(client_fd);
        return false;
    }

    const std::string body = "Hello from webserv v0.1.0\n";
    const std::string response =
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "Connection: close\r\n\r\n" + body;

    ssize_t sent = send(client_fd, response.c_str(), response.size(), 0);
    if (sent < 0) {
        std::cerr << "응답 송신 실패: " << std::strerror(errno) << std::endl;
        ::close(client_fd);
        return false;
    }

    ::close(client_fd);
    return true;
}

}  // namespace

int main(int argc, char *argv[]) {
    uint16_t port = 8080;
    if (argc >= 2) {
        port = static_cast<uint16_t>(std::atoi(argv[1]));
    }

    int listen_fd = createListenSocket(port);
    if (listen_fd < 0) {
        return EXIT_FAILURE;
    }

    bool ok = respondOnce(listen_fd);
    ::close(listen_fd);

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
