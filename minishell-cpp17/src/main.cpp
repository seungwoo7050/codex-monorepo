/**
 * [모듈] minishell-cpp17/src/main.cpp
 * 설명:
 *   - 환경 변수 확장과 빌트인을 지원하는 단일 라인 셸 엔트리포인트를 제공한다.
 *   - 공백 기반 토큰화 후 빌트인을 우선 처리하고, 외부 명령은 자식 프로세스로 실행한다.
 * 버전: v0.2.0
 * 관련 설계문서:
 *   - design/minishell-cpp17/v0.1.0-minimal-shell.md
 *   - design/minishell-cpp17/v0.2.0-env-and-builtins.md
 * 변경 이력:
 *   - v0.1.0: 단일 명령 실행과 종료 코드 출력 기능 추가
 *   - v0.2.0: 환경 변수 확장, cd/exit/env 빌트인 추가 및 종료 코드 전달
 * 테스트:
 *   - tests/run_echo.sh
 *   - tests/env_expansion.sh
 *   - tests/builtin_cd_env.sh
 *   - tests/builtin_exit_status.sh
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <unistd.h>

extern char **environ;

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
 * expandVariables
 * 설명:
 *   - 입력 문자열에서 `$VAR` 형태를 찾아 환경 변수 값으로 확장한다.
 * 입력:
 *   - line: 사용자가 입력한 원본 문자열
 * 출력:
 *   - 환경 변수가 치환된 새로운 문자열
 * 에러:
 *   - 지원하지 않는 식별자 형식은 그대로 유지된다.
 * 관련 설계문서:
 *   - design/minishell-cpp17/v0.2.0-env-and-builtins.md
 * 관련 테스트:
 *   - tests/env_expansion.sh
 */
std::string expandVariables(const std::string &line) {
    std::string expanded;
    expanded.reserve(line.size());

    for (std::size_t i = 0; i < line.size(); ++i) {
        if (line[i] != '$') {
            expanded.push_back(line[i]);
            continue;
        }

        std::size_t start = i + 1;
        if (start >= line.size() || !(std::isalpha(line[start]) || line[start] == '_')) {
            expanded.push_back(line[i]);
            continue;
        }

        std::size_t end = start;
        while (end < line.size() && (std::isalnum(line[end]) || line[end] == '_')) {
            ++end;
        }

        std::string key = line.substr(start, end - start);
        const char *value = std::getenv(key.c_str());
        if (value != nullptr) {
            expanded.append(value);
        }
        i = end - 1;
    }

    return expanded;
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

/**
 * runBuiltin
 * 설명:
 *   - cd/env/exit 빌트인을 처리한다. exit의 경우 프로그램 종료 여부를 함께 반환한다.
 * 입력:
 *   - args: 명령과 인자 목록
 * 출력:
 *   - 빌트인을 실행했다면 true, 아니면 false
 * 에러:
 *   - 잘못된 인자가 전달되면 한국어 오류 메시지를 출력하고 종료 코드를 1로 설정한다.
 * 관련 설계문서:
 *   - design/minishell-cpp17/v0.2.0-env-and-builtins.md
 * 관련 테스트:
 *   - tests/builtin_cd_env.sh
 *   - tests/builtin_exit_status.sh
 */
bool runBuiltin(const std::vector<std::string> &args, bool &should_exit, int &exit_code) {
    should_exit = false;
    if (args.empty()) {
        return false;
    }

    const std::string &command = args[0];
    if (command == "cd") {
        const char *target = nullptr;
        if (args.size() >= 2) {
            target = args[1].c_str();
        } else {
            target = std::getenv("HOME");
        }

        if (target == nullptr) {
            std::cerr << "cd 대상 경로를 찾을 수 없습니다." << std::endl;
            exit_code = 1;
            return true;
        }

        if (chdir(target) != 0) {
            std::cerr << "디렉터리 이동 실패: " << std::strerror(errno) << std::endl;
            exit_code = 1;
            return true;
        }

        char cwd[4096];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            std::cout << "현재 디렉터리: " << cwd << std::endl;
        }
        exit_code = 0;
        return true;
    }

    if (command == "env") {
        for (char **env = environ; *env != nullptr; ++env) {
            std::cout << *env << std::endl;
        }
        exit_code = 0;
        return true;
    }

    if (command == "exit") {
        if (args.size() >= 2) {
            char *end = nullptr;
            long parsed = std::strtol(args[1].c_str(), &end, 10);
            if (*args[1].c_str() != '\0' && end != nullptr && *end == '\0') {
                exit_code = static_cast<int>(parsed);
            } else {
                std::cerr << "종료 코드가 올바르지 않습니다." << std::endl;
                exit_code = 1;
            }
        } else {
            exit_code = 0;
        }
        should_exit = true;
        return true;
    }

    return false;
}

}  // namespace

int main() {
    std::string line;

    std::cout << "$ " << std::flush;
    if (!std::getline(std::cin, line)) {
        std::cerr << "입력을 읽을 수 없습니다." << std::endl;
        return EXIT_FAILURE;
    }

    std::string expanded = expandVariables(line);
    std::vector<std::string> args = splitArguments(expanded);
    if (args.empty()) {
        std::cerr << "실행할 명령이 없습니다." << std::endl;
        return EXIT_SUCCESS;
    }

    bool should_exit = false;
    int builtin_exit = 0;
    if (runBuiltin(args, should_exit, builtin_exit)) {
        if (should_exit) {
            return builtin_exit;
        }

        std::cout << "exit status: " << builtin_exit << std::endl;
        return builtin_exit;
    }

    int exit_code = executeCommand(args);
    if (exit_code < 0) {
        return EXIT_FAILURE;
    }

    std::cout << "exit status: " << exit_code << std::endl;
    return exit_code;
}
