# webserv-cpp17 v1.0.0

## 개요
C++17 단일 스레드 이벤트 루프로 동작하는 HTTP/1.1 서버이다. v1.0.0에서는 다중 연결, Host 헤더·keep-alive 처리, 동적 핸들러(`/health`, `/metrics`)를 포함한 구성을 정리해 포트폴리오용으로 문서화했다.

## 주요 기능
- `select` 기반 논블로킹 이벤트 루프와 연결 타임아웃 관리
- HTTP/1.1 파서: 요청 라인, 헤더, 본문 처리 및 Host 필수 검사
- keep-alive 지속 연결 지원, 단일 소켓에서 여러 요청 순차 처리
- 정적 파일 서빙과 404 응답 처리
- 경로 기반 핸들러 등록으로 `/health`, `/metrics` 등 동적 콘텐츠 제공

## 빌드
```bash
cmake -S webserv-cpp17 -B webserv-cpp17/build
cmake --build webserv-cpp17/build
```

## 실행
```bash
./webserv-cpp17/build/webserv 8080 3
```
- 첫 번째 인자는 포트, 두 번째 인자는 동시에 처리할 최대 요청 개수이다(테스트 기본값 3).
- `configs/dev.conf` 예제를 참고해 루트 디렉터리와 핸들러를 조정할 수 있다.

## 테스트
```bash
ctest --test-dir webserv-cpp17/build
```
- 다중 연결, Host 헤더 검증, keep-alive, 동적 핸들러 응답을 검증한다.

## 설계 문서
- 최종 개요: `design/webserv-cpp17/v1.0.0-overview.md`
- 버전별 상세 설계: `design/webserv-cpp17/` 이하 파일 참조

## 아키텍처 요약
- **이벤트 루프**: 서버 소켓과 클라이언트 소켓을 `select`로 감시하고, 타임아웃 시 연결을 정리한다.
- **요청 파서**: 상태 머신으로 요청 라인→헤더→본문 순서로 읽어 완전한 요청 객체를 만든다.
- **응답기**: 정적 파일은 파일 시스템에서 읽어 200/404로 응답하고, 등록된 핸들러는 동적으로 바디를 생성한다.
- **연결 관리**: `Connection` 구조체에서 요청 상태, keep-alive 여부, 마지막 활동 시각을 관리한다.
