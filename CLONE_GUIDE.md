# CLONE_GUIDE

## 개요
이 저장소는 네 개의 프로젝트(minishell-cpp17, webserv-cpp17, infra-inception, philosophers-cpp17)를 포함한다. 모든 주석과 문서는 한국어로 작성되어 있으며, 빌드/테스트 방법은 아래를 따른다. 각 프로젝트는 v1.0.0 기준 포트폴리오용으로 정리되어 있으며, 세부 설명은 프로젝트별 README/설계 문서를 참고한다.

## 공통 요구사항
- CMake 3.16 이상
- C++17 지원 컴파일러
- curl, Python 3 (webserv 테스트용)
- Docker, Docker Compose (infra-inception 실행용)

## minishell-cpp17
- README: `minishell-cpp17/README.md`에서 기능과 실행 예시를 요약했다.
- 빌드
  ```bash
  cmake -S minishell-cpp17 -B minishell-cpp17/build
  cmake --build minishell-cpp17/build
  ```
- 테스트
  ```bash
  ctest --test-dir minishell-cpp17/build
  ```
- 주요 옵션: 인터랙티브 루프에서 환경 변수 확장, cd/exit/env 빌트인, 파이프(`|`)와 리다이렉션(`<`, `>`, `>>`)을 지원한다. Ctrl+C로 현재 작업을 중단하고 Ctrl+D(EOF)로 종료할 수 있다.

## webserv-cpp17
- README: `webserv-cpp17/README.md`에서 이벤트 루프와 핸들러 구성을 설명한다.
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
- select 기반 이벤트 루프를 사용하며, Host 헤더 검증과 keep-alive를 지원한다. 타임아웃은 약 1.5초로 설정되어 있고, `/health`, `/metrics` 동적 엔드포인트를 기본 제공한다.

## infra-inception
- README: `infra-inception/README.md`에서 서비스 구성과 운영 메모를 확인한다.
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
- 구성: app(db/redis 의존), db(MariaDB), redis, nginx(8080 포트 프록시) 서비스를 포함한다. 모든 서비스는 `TZ=Asia/Seoul` 환경을 사용하며 DB는 `utf8mb4` 설정, Nginx는 gzip/시간 로그 포맷으로 튜닝되어 있다. 정적 파일은 `services/nginx/static/`에서 제공되며, Prometheus(9090)와 Grafana(3000) 모니터링 스택이 추가되었다.

## philosophers-cpp17
- 설명: 고전 식사하는 철학자 문제를 C++17 스레드/뮤텍스로 구현한 학습용 데모이다. v1.0.0에서는 전략(naive/ordered/waiter), 실행 제어, 보고 단계를 분리하고 `--help`/설정 검증을 통해 포트폴리오용 CLI를 제공한다.
- 빌드
  ```bash
  cmake -S philosophers-cpp17 -B philosophers-cpp17/build
  cmake --build philosophers-cpp17/build
  ```
- 실행 예시
  ```bash
  # 도움말: 사용 가능한 옵션을 한국어로 요약한다.
  ./philosophers-cpp17/build/philosophers --help

  # naive 전략: 교착 징후 메시지가 출력된다.
  ./philosophers-cpp17/build/philosophers --duration-ms 2200 --lock-timeout-ms 1000 --stuck-threshold-ms 900 --strategy naive

  # ordered 전략: 낮은 번호 포크부터 잠그는 방식으로 교착 없이 진행된다.
  ./philosophers-cpp17/build/philosophers --strategy ordered --duration-ms 1500 --think-ms 40 --eat-ms 40

  # waiter 전략: N-1 토큰 정책으로 동시에 대기하는 철학자 수를 제한한다.
  ./philosophers-cpp17/build/philosophers --strategy waiter --duration-ms 1500 --think-ms 40 --eat-ms 50

  # 공정성 지표 확인: 지터/시드를 지정해 식사 분포·최대 대기 시간을 요약한다.
  ./philosophers-cpp17/build/philosophers --strategy ordered --duration-ms 1200 --jitter-ms 10 --random-seed 42
  ```
  출력에는 교착 감지 여부(naive)와 `[요약] 전략=...` 라인이 포함되며 철학자별 식사 횟수, 최대 대기 시간, 식사 분포 통계(평균/표준편차 등)를 확인할 수 있다. 음수 시간/1인 구성 등 잘못된 설정은 즉시 오류 메시지를 반환한다.
- 테스트
  ```bash
  ctest --test-dir philosophers-cpp17/build --output-on-failure
  ```
