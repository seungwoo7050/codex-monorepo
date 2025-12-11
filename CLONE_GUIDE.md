# CLONE_GUIDE

## 공통
- 빌드 도구: CMake 3.16+, g++(C++17), curl, bash.
- 저장소 클론 후 각 프로젝트별 빌드/실행 방법을 따른다.

## minishell-cpp17 v0.1.0
- 빌드: `cmake -S minishell-cpp17 -B minishell-cpp17/build && cmake --build minishell-cpp17/build`
- 실행: `echo "echo hi" | ./minishell-cpp17/build/minishell`
- 테스트: `cd minishell-cpp17 && ctest --test-dir build`

## webserv-cpp17 v0.1.0
- 빌드: `cmake -S webserv-cpp17 -B webserv-cpp17/build && cmake --build webserv-cpp17/build`
- 실행: `./webserv-cpp17/build/webserv 8080`
- 테스트: `cd webserv-cpp17 && ctest --test-dir build`

## infra-inception v0.1.0
- 실행: `cd infra-inception && docker compose up -d`
- 종료: `docker compose down`
- 테스트: `cd infra-inception && ./tests/compose_lint.sh`
