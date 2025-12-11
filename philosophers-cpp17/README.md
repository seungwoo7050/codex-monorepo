# philosophers-cpp17 (v1.0.0)

## 개요
- 고전 식사하는 철학자 문제를 C++17 스레드/뮤텍스로 구현한 학습용 시뮬레이터이다.
- naive/ordered/waiter 전략을 동일한 실행 파일에서 비교할 수 있으며, 공정성 통계와 교착 의심 로그를 제공한다.
- 설정 파싱, 실행 제어, 보고 단계를 분리해 포트폴리오용 구조와 한국어 CLI 도움말을 갖췄다.

## 빌드
```bash
cmake -S . -B build
cmake --build build
```

## 실행 예시
```bash
# 도움말: 사용 가능한 옵션 요약
./build/philosophers --help

# naive 전략(교착 데모)
./build/philosophers --duration-ms 2200 --lock-timeout-ms 1000 --stuck-threshold-ms 900 --strategy naive

# ordered 전략(교착 회피)
./build/philosophers --strategy ordered --duration-ms 1500 --think-ms 40 --eat-ms 40

# waiter 전략(토큰 기반 진입 제한)
./build/philosophers --strategy waiter --duration-ms 1500 --think-ms 40 --eat-ms 50

# 공정성 지표 확인(지터/시드 지정)
./build/philosophers --strategy ordered --duration-ms 1200 --jitter-ms 10 --random-seed 42
```

## 주요 옵션
- `--philosophers <N>`: 철학자/포크 수 (기본 5, 2 이상 필수)
- `--strategy naive|ordered|waiter`: 전략 선택
- `--think-ms`, `--eat-ms`: 생각/식사 시간 조정
- `--lock-timeout-ms`: 포크 대기 타임아웃
- `--stuck-threshold-ms`: 교착 의심 임계값
- `--duration-ms`: 전체 실행 시간 (0보다 커야 함)
- `--jitter-ms`: 시작/슬립 지터 범위
- `--random-seed`: RNG 시드
- `--help`/`-h`: 옵션 요약 출력

## 실행 흐름 요약
1. `parseArguments`에서 CLI 인자를 파싱하고 `validateConfig`로 음수 시간/인원 부족/0ms 실행을 차단한다.
2. `run`이 철학자 스레드와 모니터 스레드를 기동하고 설정 요약을 로깅한다.
3. 각 전략 함수(`acquireNaive`, `acquireOrdered`, `acquireWaiter`)가 포크 잠금 순서를 정의한다.
4. `summarize`/`logSummary`가 식사 횟수, 최대 대기 시간, 분포(평균/표준편차)를 보고한다.

## 테스트
```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## 참고
- 설계 문서: `design/philosophers-cpp17/v1.0.0-overview.md`
- 이전 버전의 세부 전략 변화는 `design/philosophers-cpp17/` 이하 문서를 참고한다.
