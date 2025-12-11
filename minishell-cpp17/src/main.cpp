/**
 * [모듈] minishell-cpp17/src/main.cpp
 * 설명:
 *   - 기본 명령어 한 줄을 읽어 공백 기준으로 분리하고 자식 프로세스로 실행한다.
 *   - 프로세스 종료 코드를 출력하여 사용자가 결과를 확인할 수 있도록 한다.
 * 버전: v0.1.0
 * 관련 설계문서:
 *   - design/minishell-cpp17/v0.1.0-minimal-shell.md
 * 변경 이력:
 *   - v0.1.0: 단일 명령 실행과 종료 코드 출력 기능 추가
 * 테스트:
 *   - tests/run_echo.sh
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

/**
 * splitArguments
 * 설명:
 *   - 공백을 기준으로 사용자가 입력한 명령줄을 분리한다.
 * 입력:
 *   - line: 사용자가 입력한 전체 문자열
 * 출력:
 *   - 공백으로 나뉜 토큰 벡터. 비어있을 수 있다.
 * 에러:
 *   - 토큰화 과정에서는 별도의 예외 상황이 없다.
 * 관련 설계문서:
 *   - design/minishell-cpp17/v0.1.0-minimal-shell.md
 * 관련 테스트:
 *   - tests/run_echo.sh
 */
std::vector<std::string> splitArguments(const std::string &line) {
    std::istringstream stream(line);
    std::vector<std::string> tokens;
    std::string token;

    while (stream >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

/**
 * executeCommand
 * 설명:
 *   - 토큰화된 명령어를 fork 후 execvp로 실행한다.
 * 입력:
 *   - args: 프로그램 이름을 포함한 인자 리스트
 * 출력:
 *   - 자식 프로세스의 종료 코드를 반환한다. 자식 생성 실패 시 -1을 반환한다.
 * 에러:
 *   - fork 실패 시 -1을 반환하고 stderr에 오류 메시지를 남긴다.
 *   - execvp 실패 시 자식 프로세스가 종료되며 부모는 비정상 종료 코드를 확인한다.
 * 관련 설계문서:
 *   - design/minishell-cpp17/v0.1.0-minimal-shell.md
 * 관련 테스트:
 *   - tests/run_echo.sh
 */
int executeCommand(const std::vector<std::string> &args) {
    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "프로세스 생성 실패: " << std::strerror(errno) << std::endl;
        return -1;
    }

    if (pid == 0) {
        std::vector<char *> argv;
        argv.reserve(args.size() + 1);
        for (std::vector<std::string>::size_type i = 0; i < args.size(); ++i) {
            argv.push_back(const_cast<char *>(args[i].c_str()));
        }
        argv.push_back(nullptr);
        execvp(argv[0], argv.data());
        std::cerr << "명령 실행 실패: " << std::strerror(errno) << std::endl;
        std::_Exit(127);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        std::cerr << "프로세스 대기 실패: " << std::strerror(errno) << std::endl;
        return -1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }

    return -1;
}

}  // namespace

int main() {
    std::string line;

    std::cout << "$ " << std::flush;
    if (!std::getline(std::cin, line)) {
        std::cerr << "입력을 읽을 수 없습니다." << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<std::string> args = splitArguments(line);
    if (args.empty()) {
        std::cerr << "실행할 명령이 없습니다." << std::endl;
        return EXIT_SUCCESS;
    }

    int exit_code = executeCommand(args);
    if (exit_code < 0) {
        return EXIT_FAILURE;
    }

    std::cout << "exit status: " << exit_code << std::endl;
    return EXIT_SUCCESS;
}
