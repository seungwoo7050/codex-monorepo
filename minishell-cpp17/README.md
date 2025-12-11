# minishell-cpp17 v1.0.0

## 개요
C++17로 작성된 단일 스레드 POSIX 스타일 셸 구현이다. v1.0.0에서는 v0.1.0~v0.4.0에서 개발한 기능을 정리하고 문서화하여 포트폴리오 용도로 안정화했다. 파이프와 리다이렉션, 환경 변수 확장, cd/exit/env 빌트인, Ctrl+C/EOF 처리 등 기본 셸 동작을 모두 제공한다.

## 주요 기능
- 공백 기반 토큰화와 파이프(`|`), 리다이렉션(`<`, `>`, `>>`) 구문 파싱
- `$VAR` 환경 변수 확장
- 빌트인 명령어: `cd`, `exit`, `env`
- `fork`/`execvp` 기반 실행과 파이프라인 파일 디스크립터 RAII 정리
- Ctrl+C로 현재 작업만 중단하고 셸은 유지, EOF(Ctrl+D)로 종료

## 빌드
```bash
cmake -S minishell-cpp17 -B minishell-cpp17/build
cmake --build minishell-cpp17/build
```

## 실행
```bash
./minishell-cpp17/build/minishell
```
- 프롬프트는 `$ ` 형태로 출력되며, 명령 실행 후 종료 코드를 표시한다.

## 테스트
```bash
ctest --test-dir minishell-cpp17/build
```
- 테스트 스크립트는 기본 명령 실행, 환경 변수 확장, 빌트인, 파이프/리다이렉션, 시그널/EOF 시나리오를 포함한다.

## 설계 문서
- 최종 개요: `design/minishell-cpp17/v1.0.0-overview.md`
- 하위 버전별 상세 설계: `design/minishell-cpp17/` 이하 파일 참조

## 아키텍처 요약
- 파서: 토큰화 후 파이프/리다이렉션 정보를 `Command` 목록으로 변환한다.
- 실행기: 파이프라인을 순차적으로 `fork`하여 그룹화하고, 리다이렉션을 설정한 뒤 `execvp`로 실행한다.
- 빌트인 처리기: 파싱 결과가 단일 명령일 때 우선 처리해 별도 프로세스를 만들지 않는다.
- 시그널 처리: `sigaction(SIGINT)`으로 인터럽트 플래그를 관리하고 진행 중인 자식 프로세스 그룹에 전달한다.
