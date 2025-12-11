# CLONE_GUIDE

## 개요
이 저장소는 세 개의 프로젝트(minishell-cpp17, webserv-cpp17, infra-inception)를 포함한다. 모든 주석과 문서는 한국어로 작성되어 있으며, 빌드/테스트 방법은 아래를 따른다.

## 공통 요구사항
- CMake 3.16 이상
- C++17 지원 컴파일러
- curl, Python 3 (webserv 테스트용)
- Docker, Docker Compose (infra-inception 실행용)

## minishell-cpp17
- 빌드
  ```bash
  cmake -S minishell-cpp17 -B minishell-cpp17/build
  cmake --build minishell-cpp17/build
  ```
- 테스트
  ```bash
  ctest --test-dir minishell-cpp17/build
  ```
- 주요 옵션: 단일 입력 라인 기반으로 환경 변수 확장, cd/exit/env 빌트인, 파이프(`|`)와 리다이렉션(`<`, `>`, `>>`)을 지원한다.

## webserv-cpp17
- 빌드
  ```bash
  cmake -S webserv-cpp17 -B webserv-cpp17/build
  cmake --build webserv-cpp17/build
  ```
- 테스트
  ```bash
  ctest --test-dir webserv-cpp17/build
  ```
- 실행 예시: 기본값으로 8080 포트, 최대 처리 요청 3건
  ```bash
  ./webserv-cpp17/build/webserv 8080 3
  ```
- select 기반 이벤트 루프를 사용하며, Host 헤더 검증과 keep-alive를 지원한다. 타임아웃은 약 1.5초로 설정되어 있다.

## infra-inception
- 요구사항: Docker, Docker Compose
- 실행
  ```bash
  cd infra-inception
  docker compose up -d
  ```
- 중지
  ```bash
  cd infra-inception
  docker compose down
  ```
- 구성: app(db/redis 의존), db(MariaDB), redis, nginx(8080 포트 프록시) 서비스를 포함한다. 모든 서비스는 `TZ=Asia/Seoul` 환경을 사용하며 DB는 `utf8mb4` 설정, Nginx는 gzip/시간 로그 포맷으로 튜닝되어 있다. 정적 파일은 `services/nginx/static/`에서 제공된다.
