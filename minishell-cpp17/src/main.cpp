/**
 * [모듈] minishell-cpp17/src/main.cpp
 * 설명:
 *   - 파이프라인, 리다이렉션을 포함한 단일 쓰레드 셸 루프를 실행한다.
 *   - v0.4.0에서 시그널 처리(Ctrl+C, Ctrl+D)와 구조화된 오류 보고를 강화한다.
 * 버전: v0.4.0
 * 관련 설계문서:
 *   - design/minishell-cpp17/v0.1.0-minimal-shell.md
 *   - design/minishell-cpp17/v0.2.0-env-and-builtins.md
 *   - design/minishell-cpp17/v0.3.0-pipelines-and-redirections.md
 *   - design/minishell-cpp17/v0.4.0-signals-and-errors.md
 * 변경 이력:
 *   - v0.1.0: 단일 명령 실행과 종료 코드 출력 기능 추가
 *   - v0.2.0: 환경 변수 확장, cd/exit/env 빌트인 추가 및 종료 코드 전달
 *   - v0.3.0: 파이프, 입력/출력(append) 리다이렉션 지원 및 종료 코드 전달 개선
 *   - v0.4.0: Ctrl+C/Ctrl+D 대응, 오류 구조화, 인터랙티브 루프화
 * 테스트:
 *   - tests/run_echo.sh
 *   - tests/env_expansion.sh
 *   - tests/builtin_cd_env.sh
 *   - tests/builtin_exit_status.sh
 *   - tests/pipeline_basic.sh
 *   - tests/redirection_basic.sh
 *   - tests/signal_interrupt.sh
 *   - tests/eof_exit.sh
 */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cctype>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

extern char **environ;

namespace {

struct ParseError {
    std::string message;
};

struct ExecutionError {
    std::string message;
    int         exit_code;
};

struct Command {
    std::vector<std::string> args;
    std::optional<std::string> input_file;
    std::optional<std::string> output_file;
    bool append_output;
};

volatile sig_atomic_t g_interrupted = 0;
volatile sig_atomic_t g_child_group = -1;

void handleSigInt(int) {
    g_interrupted = 1;
    if (g_child_group > 0) {
        kill(-g_child_group, SIGINT);
    }
}

/**
 * splitArguments
 * 설명:
 *   - 공백과 연산자를 기준으로 사용자가 입력한 명령줄을 분리한다.
 * 입력:
 *   - line: 사용자가 입력한 전체 문자열
 * 출력:
 *   - 공백/연산자로 나뉜 토큰 벡터. 비어있을 수 있다.
 * 에러:
 *   - 토큰화 과정에서는 별도의 예외 상황이 없다.
 * 관련 설계문서:
 *   - design/minishell-cpp17/v0.1.0-minimal-shell.md
 * 관련 테스트:
 *   - tests/run_echo.sh
 */
std::vector<std::string> splitArguments(const std::string &line) {
    std::vector<std::string> tokens;
    std::string current;

    auto flush_token = [&]() {
        if (!current.empty()) {
            tokens.push_back(current);
            current.clear();
        }
    };

    for (std::size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (std::isspace(static_cast<unsigned char>(c))) {
            flush_token();
            continue;
        }

        if (c == '|' || c == '<' || c == '>') {
            flush_token();
            if (c == '>' && i + 1 < line.size() && line[i + 1] == '>') {
                tokens.push_back(">");
                tokens.back().push_back('>');
                ++i;
            } else {
                tokens.push_back(std::string(1, c));
            }
            continue;
        }

        current.push_back(c);
    }

    flush_token();
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
        char *value = std::getenv(key.c_str());
        if (value) {
            expanded.append(value);
        }
        i = end - 1;
    }

    return expanded;
}

/**
 * runBuiltin
 * 설명:
 *   - cd/exit/env 빌트인을 처리한다.
 * 입력:
 *   - args: 명령어와 인자를 포함한 벡터
 *   - should_exit: exit 호출 여부 출력 플래그
 *   - exit_code: exit 코드 또는 빌트인 결과 코드
 * 출력:
 *   - 빌트인 처리 여부를 반환한다.
 * 에러:
 *   - cd 실패, 잘못된 exit 인자 등에서 한국어 오류 메시지를 출력한다.
 * 관련 설계문서:
 *   - design/minishell-cpp17/v0.2.0-env-and-builtins.md
 * 관련 테스트:
 *   - tests/builtin_cd_env.sh
 *   - tests/builtin_exit_status.sh
 */
bool runBuiltin(const std::vector<std::string> &args, bool &should_exit, int &exit_code) {
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
            if (!target) {
                std::cerr << "HOME 환경 변수가 설정되어 있지 않습니다." << std::endl;
                return true;
            }
        }

        if (chdir(target) != 0) {
            std::cerr << "디렉터리 이동 실패: " << std::strerror(errno) << std::endl;
            exit_code = 1;
        } else {
            char cwd_buf[4096];
            if (getcwd(cwd_buf, sizeof(cwd_buf))) {
                std::cout << "현재 디렉터리: " << cwd_buf << std::endl;
            }
            exit_code = 0;
        }
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

/**
 * parsePipeline
 * 설명:
 *   - 토큰 목록을 파이프라인 명령 구조로 변환한다.
 * 입력:
 *   - tokens: 공백/연산자 단위로 분리된 토큰 목록
 *   - error_out: 파싱 실패 시 메시지를 담는 구조체
 * 출력:
 *   - 파이프/리다이렉션 정보가 포함된 명령 벡터. 실패 시 std::nullopt.
 * 에러:
 *   - 리다이렉션 대상이 없거나 파이프 양끝이 비어 있을 때 오류 메시지를 error_out에 기록한다.
 * 관련 설계문서:
 *   - design/minishell-cpp17/v0.3.0-pipelines-and-redirections.md
 *   - design/minishell-cpp17/v0.4.0-signals-and-errors.md
 * 관련 테스트:
 *   - tests/pipeline_basic.sh
 *   - tests/redirection_basic.sh
 */
std::optional<std::vector<Command> > parsePipeline(const std::vector<std::string> &tokens, ParseError &error_out) {
    std::vector<Command> commands;
    Command current;
    current.append_output = false;

    auto push_current = [&]() -> bool {
        if (current.args.empty()) {
            error_out.message = "파이프의 한쪽 명령이 비어 있습니다.";
            return false;
        }
        commands.push_back(current);
        current = Command();
        current.append_output = false;
        return true;
    };

    for (std::size_t i = 0; i < tokens.size(); ++i) {
        const std::string &token = tokens[i];
        if (token == "|") {
            if (!push_current()) {
                return std::nullopt;
            }
            continue;
        }

        if (token == "<" || token == ">" || token == ">>") {
            if (i + 1 >= tokens.size()) {
                error_out.message = "리다이렉션 대상이 누락되었습니다.";
                return std::nullopt;
            }
            const std::string &target = tokens[i + 1];
            if (token == "<") {
                current.input_file = target;
            } else {
                current.output_file = target;
                current.append_output = (token == ">>");
            }
            ++i;
            continue;
        }

        current.args.push_back(token);
    }

    if (!current.args.empty()) {
        if (!push_current()) {
            return std::nullopt;
        }
    }

    if (commands.empty()) {
        error_out.message = "실행할 명령이 없습니다.";
        return std::nullopt;
    }

    return commands;
}

/**
 * setupRedirection
 * 설명:
 *   - 입력/출력 리다이렉션을 위해 파일을 열고 FD를 교체한다.
 * 입력:
 *   - cmd: 리다이렉션 정보가 포함된 명령 구조체
 * 출력:
 *   - 성공 시 true, 실패 시 false
 * 에러:
 *   - 파일 열기/dup2 실패 시 한국어 오류 메시지를 남기고 false를 반환한다.
 * 관련 설계문서:
 *   - design/minishell-cpp17/v0.3.0-pipelines-and-redirections.md
 * 관련 테스트:
 *   - tests/redirection_basic.sh
 */
bool setupRedirection(const Command &cmd) {
    if (cmd.input_file.has_value()) {
        int fd = open(cmd.input_file->c_str(), O_RDONLY);
        if (fd < 0) {
            std::cerr << "입력 파일을 열 수 없습니다: " << std::strerror(errno) << std::endl;
            return false;
        }
        if (dup2(fd, STDIN_FILENO) < 0) {
            std::cerr << "표준 입력 대체 실패: " << std::strerror(errno) << std::endl;
            close(fd);
            return false;
        }
        close(fd);
    }

    if (cmd.output_file.has_value()) {
        int flags = O_WRONLY | O_CREAT;
        flags |= cmd.append_output ? O_APPEND : O_TRUNC;
        int fd = open(cmd.output_file->c_str(), flags, 0644);
        if (fd < 0) {
            std::cerr << "출력 파일을 열 수 없습니다: " << std::strerror(errno) << std::endl;
            return false;
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            std::cerr << "표준 출력 대체 실패: " << std::strerror(errno) << std::endl;
            close(fd);
            return false;
        }
        close(fd);
    }

    return true;
}

/**
 * executePipeline
 * 설명:
 *   - 파싱된 명령 벡터를 순차적으로 파이프 연결 후 실행한다.
 * 입력:
 *   - commands: 파이프/리다이렉션 정보가 포함된 명령 목록
 *   - error_out: 실행 실패 시 메시지와 종료 코드를 담는 구조체
 * 출력:
 *   - 성공 시 마지막 프로세스 종료 코드를 반환, 실패 시 std::nullopt
 * 에러:
 *   - fork/pipe/exec 실패 시 한국어 오류 메시지를 출력하고 error_out에 기록한다.
 * 관련 설계문서:
 *   - design/minishell-cpp17/v0.3.0-pipelines-and-redirections.md
 *   - design/minishell-cpp17/v0.4.0-signals-and-errors.md
 * 관련 테스트:
 *   - tests/pipeline_basic.sh
 *   - tests/redirection_basic.sh
 *   - tests/signal_interrupt.sh
 */
std::optional<int> executePipeline(const std::vector<Command> &commands, ExecutionError &error_out) {
    std::vector<pid_t> children;
    std::vector<int> pipes;

    if (commands.size() > 1) {
        pipes.resize((commands.size() - 1) * 2, -1);
        for (std::size_t i = 0; i + 1 < commands.size(); ++i) {
            if (pipe(&pipes[i * 2]) < 0) {
                error_out.message = std::string("파이프 생성 실패: ") + std::strerror(errno);
                error_out.exit_code = 1;
                for (int fd : pipes) {
                    if (fd >= 0) close(fd);
                }
                return std::nullopt;
            }
        }
    }

    pid_t group_leader = -1;

    for (std::size_t idx = 0; idx < commands.size(); ++idx) {
        pid_t pid = fork();
        if (pid < 0) {
            error_out.message = std::string("프로세스 생성 실패: ") + std::strerror(errno);
            error_out.exit_code = 1;
            return std::nullopt;
        }

        if (pid == 0) {
            if (group_leader == -1) {
                setpgid(0, 0);
            } else {
                setpgid(0, group_leader);
            }

            if (commands.size() > 1) {
                if (idx > 0) {
                    int in_fd = pipes[(idx - 1) * 2];
                    dup2(in_fd, STDIN_FILENO);
                }
                if (idx + 1 < commands.size()) {
                    int out_fd = pipes[idx * 2 + 1];
                    dup2(out_fd, STDOUT_FILENO);
                }
                for (int fd : pipes) {
                    if (fd >= 0) close(fd);
                }
            }

            if (!setupRedirection(commands[idx])) {
                _exit(EXIT_FAILURE);
            }

            std::vector<char *> argv;
            argv.reserve(commands[idx].args.size() + 1);
            for (const std::string &arg : commands[idx].args) {
                argv.push_back(const_cast<char *>(arg.c_str()));
            }
            argv.push_back(nullptr);

            execvp(argv[0], argv.data());
            std::cerr << "명령 실행 실패: " << std::strerror(errno) << std::endl;
            _exit(127);
        }

        if (group_leader == -1) {
            group_leader = pid;
            setpgid(pid, group_leader);
        } else {
            setpgid(pid, group_leader);
        }

        children.push_back(pid);
    }

    g_child_group = group_leader;

    for (int fd : pipes) {
        if (fd >= 0) close(fd);
    }

    int status = 0;
    int last_exit = 0;
    for (pid_t child : children) {
        if (waitpid(child, &status, 0) < 0) {
            if (errno == EINTR && g_interrupted) {
                last_exit = 130;
                break;
            }
            error_out.message = std::string("자식 프로세스 대기 실패: ") + std::strerror(errno);
            error_out.exit_code = 1;
            g_child_group = -1;
            return std::nullopt;
        }
        if (WIFEXITED(status)) {
            last_exit = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            last_exit = 128 + WTERMSIG(status);
        }
    }

    g_child_group = -1;
    return last_exit;
}

}  // namespace

int main() {
    struct sigaction sa = {};
    sa.sa_handler = handleSigInt;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, nullptr);

    std::string line;
    int last_status = 0;

    while (true) {
        std::cout << "$ " << std::flush;
        if (!std::getline(std::cin, line)) {
            std::cout << std::endl;
            break;
        }

        if (g_interrupted) {
            g_interrupted = 0;
            std::cout << std::endl;
            continue;
        }

        std::string expanded = expandVariables(line);
        std::vector<std::string> tokens = splitArguments(expanded);
        if (tokens.empty()) {
            continue;
        }

        std::optional<std::vector<Command> > parsed;
        ParseError parse_error;
        parsed = parsePipeline(tokens, parse_error);
        if (!parsed.has_value()) {
            std::cerr << "파싱 오류: " << parse_error.message << std::endl;
            last_status = 2;
            continue;
        }

        const std::vector<Command> &commands = parsed.value();
        if (commands.size() == 1) {
            bool should_exit = false;
            int builtin_exit = 0;
            if (runBuiltin(commands[0].args, should_exit, builtin_exit)) {
                last_status = builtin_exit;
                if (should_exit) {
                    break;
                }
                std::cout << "exit status: " << builtin_exit << std::endl;
                continue;
            }
        }

        ExecutionError exec_error;
        std::optional<int> exit_code = executePipeline(commands, exec_error);
        if (!exit_code.has_value()) {
            std::cerr << "실행 오류: " << exec_error.message << std::endl;
            last_status = exec_error.exit_code;
            continue;
        }

        if (g_interrupted) {
            std::cout << std::endl;
            g_interrupted = 0;
        }

        last_status = exit_code.value();
        std::cout << "exit status: " << last_status << std::endl;
    }

    return last_status;
}
