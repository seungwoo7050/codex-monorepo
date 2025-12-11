# DOC_TEMPLATES.md

This document provides templates and examples for:

- Source code comments (for learning/teaching purposes)
- `design/` documents (initial and per-version)
- `CLONE_GUIDE.md`
- Consistency rules between code, design docs, and clone guides

> **IMPORTANT LANGUAGE RULE**  
> - All comments in source code MUST be written in **Korean**.  
> - All documentation (`design/`, `CLONE_GUIDE.md`, project-specific READMEs) MUST be written in **Korean**.  
> - This file is in English only because it targets AI agents and tooling.

---

## 1. Code comment templates (Korean-only)

### 1.1 General rules

When writing or modifying code, follow these rules:

- All file/module headers MUST:
  - Be written in **Korean**.
  - Contain:
    - Module path
    - Short description
    - Current version (e.g. `v0.2.0`)
    - Related design document path(s)
    - Short change history (version + one-line summary)

- All public functions/methods MUST:
  - Have a comment block written in **Korean**.
  - Declare:
    - Purpose
    - Inputs (parameters)
    - Outputs (return value)
    - Possible errors/exceptions
    - Related tests (file or test name)
    - Related version and design doc

- Comments should **explain intent and constraints**, not repeat code literally.

---

### 1.2 File / module header comment (example, Korean)

> Use this pattern at the top of each important module/file.  
> Replace the content with the actual project/module information.

```cpp
/**
 * [모듈] minishell-cpp17/src/parser/command_parser.cpp
 * 설명:
 *   - 셸 입력 문자열을 토큰으로 분해하고, AST 형태의 명령 구조로 파싱한다.
 *   - 파이프, 리다이렉션, 따옴표 처리 등의 문법을 책임진다.
 * 버전: v0.2.0
 * 관련 설계문서:
 *   - design/minishell-cpp17/v0.2.0-parsing-and-env.md
 * 변경 이력:
 *   - v0.1.0: 단일 명령 파싱 기본 구조 추가
 *   - v0.2.0: 환경변수 확장, 파이프/리다이렉션 파싱 로직 추가
 * 테스트:
 *   - tests/parser_command_basic.cpp
 *   - tests/parser_redirection_pipe.cpp
 */
```

---

### 1.3 Function / method comment (example, Korean)

> Use this pattern for public functions/methods.
> Private helpers may use a lighter version, but comments must still be in Korean.

```cpp
/**
 * parse_command_line
 * 설명:
 *   - 한 줄의 셸 입력을 받아 토큰화 및 구문 분석을 수행하고,
 *     실행 가능한 명령 AST 구조로 변환한다.
 * 입력:
 *   - input: 사용자가 입력한 전체 셸 문자열
 * 출력:
 *   - 성공 시: std::optional<CommandAST> (유효한 AST가 존재하면 값, 없으면 std::nullopt)
 *   - 실패 시: std::nullopt를 반환하고, error_out에 오류 정보를 기록한다.
 * 에러:
 *   - 잘못된 따옴표, 지원하지 않는 연산자 등의 문법 오류가 발생할 수 있다.
 *   - 에러 상세는 error_out 구조체에 코드/메시지로 저장한다.
 * 관련 설계문서:
 *   - design/minishell-cpp17/v0.2.0-parsing-and-env.md
 * 관련 테스트:
 *   - tests/parser_command_basic.cpp
 *   - tests/parser_syntax_error.cpp
 */
std::optional<CommandAST> parse_command_line(
    const std::string& input,
    ParseError&        error_out);
```

---

### 1.4 Class / component header (example, Korean)

```cpp
/**
 * CommandExecutor (v0.3.0)
 * 역할:
 *   - 파싱된 명령 AST를 실제 프로세스 실행으로 연결하는 책임을 가진다.
 *   - 파이프/리다이렉션/빌트인 명령 실행을 통합 처리한다.
 * 설계:
 *   - design/minishell-cpp17/v0.3.0-execution-and-pipeline.md
 * 주의 사항:
 *   - execve 호출 전후로 FD 정리와 에러 처리를 확실히 해야 한다.
 *   - RAII 래퍼를 사용하여 파이프/FD 누수를 방지한다.
 */
class CommandExecutor {
  // ...
};
```

---

## 2. `design/` documents

### 2.1 Directory structure (recommended)

Design docs are grouped per project:

```text
design/
  minishell-cpp17/
    initial-design.md                     # 초기 전체 설계 (한글)
    v0.1.0-minimal-shell.md               # v0.1.0 설계 (한글)
    v0.2.0-parsing-and-env.md             # v0.2.0 설계 (한글)
    ...

  webserv-cpp17/
    initial-design.md
    v0.1.0-basic-http-server.md
    v0.2.0-multi-connection-loop.md
    ...

  infra-inception/
    initial-design.md
    v0.1.0-app-db-stack.md
    v0.2.0-nginx-redis-kor-tuning.md
    ...
```

* File names must be **English**, but the content must be **Korean**.
* Each version-related document must clearly mention its version (e.g. `v0.2.0`) at the top.

---

### 2.2 `initial-design.md` template (content in Korean)

> One `initial-design.md` per project (e.g. `design/minishell-cpp17/initial-design.md`).
> This is a high-level overview and can be updated over time.

```md
# 초기 전체 설계

## 1. 프로젝트 개요
- 이름: minishell-cpp17
- 목적:
  - POSIX 셸의 핵심 기능을 C++17로 재구현하여,
    시스템 프로그래밍과 자원 관리, 파싱/실행 구조를 학습한다.

## 2. 목표 범위
- 지원 대상:
  - 기본 명령 실행, 파이프, 리다이렉션, 환경변수, 빌트인 명령 등
- 비목표:
  - 완전한 POSIX 호환, 복잡한 job control, 고급 디버깅 기능 등은 범위 밖으로 둔다.

## 3. 아키텍처 개요
- 주요 컴포넌트:
  - Lexer/Parser: 문자열 → 토큰 → AST
  - Executor: AST → 프로세스 실행
  - Environment: 환경변수/셸 변수 관리
  - Error Handling: 에러 코드 및 메시지 구조화
- 데이터 흐름(간단 요약):
  1. 입력 읽기
  2. 토큰화 및 파싱
  3. AST 검증
  4. 명령 실행
  5. 종료 코드 반환

## 4. 버전 전략
- 버전 형식: MAJOR.MINOR.PATCH (예: v0.2.0)
- 0.x.y:
  - 기능을 확장해 가며 구조를 안정화하는 단계
- 1.0.0:
  - 포트폴리오에 공개 가능한 최소 기능 + 문서/테스트가 갖춰진 상태
- 각 버전의 상세 목표는 VERSIONING.md를 기준으로 관리한다.

## 5. 테스트 및 품질
- 단위 테스트:
  - 파서, 실행, 에러 케이스에 대한 테스트를 지속적으로 추가
- 수동 테스트:
  - 실제 셸 사용 시나리오를 기반으로 쉘 상호작용을 검증
- 코드 품질:
  - 주석과 설계 문서, CLONE_GUIDE 간의 정합성을 유지하는 것을 목표로 한다.
```

---

### 2.3 Per-version design doc template (content in Korean)

> One file per version per project (e.g. `design/webserv-cpp17/v0.2.0-multi-connection-loop.md`).

```md
# v0.2.0 - 다중 연결 이벤트 루프

## 1. 목적
- 기존 단일 연결/블로킹 방식 HTTP 서버를,
  다중 클라이언트를 처리할 수 있는 이벤트 루프 구조로 확장한다.

## 2. 변경 요약
- 기존:
  - 하나의 클라이언트에 대해 accept → read → write 순서로 블로킹 처리
- 변경:
  - non-blocking 소켓 + select/epoll 기반 루프 도입
  - 여러 클라이언트의 소켓을 동시에 감시하고, 준비된 소켓만 처리

## 3. 외부 동작 / API
- 지원 프로토콜:
  - HTTP/1.0, HTTP/1.1 (기본적인 GET/HEAD)
- 설정 파일:
  - config/webserv_v0.2.0.conf
- 동작 예시:
  - 동시에 여러 클라이언트에서 GET 요청을 보냈을 때,
    서버가 모든 요청을 정상 처리해야 한다.

## 4. 내부 설계
- 주요 구조체/클래스:
  - ServerConfig: 포트/루트 경로/타임아웃 설정
  - Connection: 각 클라이언트 소켓의 상태 및 버퍼 관리
  - EventLoop: 파일 디스크립터 집합, 타임아웃, 상태 전이 관리
- 이벤트 루프 흐름:
  1. 리스닝 소켓 준비
  2. select/epoll로 읽기/쓰기 가능 소켓 감시
  3. 신규 연결 accept
  4. 기존 연결에서 읽기/파싱/응답 생성
  5. 완료/에러/타임아웃된 연결 정리

## 5. 테스트 전략
- 자동 테스트:
  - 단일 클라이언트 요청/응답
  - 동시 다중 클라이언트 요청
  - 타임아웃/연결 종료 케이스
- 수동 테스트:
  - curl, 브라우저 등을 사용한 간단 수동 검증

## 6. 마이그레이션 / 호환성
- v0.1.0에서:
  - 설정 파일 형식을 유지하거나, 변경 시 VERSIONING.md에 명시
- 추후 버전에서:
  - HTTP/1.1 keep-alive, chunked encoding 등을 점진적으로 도입할 여지를 남겨둔다.
```

---

## 3. `CLONE_GUIDE.md` template (content in Korean)

> Root-level `CLONE_GUIDE.md` should describe how to clone, build, and run each project.
> All text must be in Korean.

````md
# CLONE_GUIDE.md

## 1. 저장소 클론

```bash
git clone <REPO_URL>
cd <REPO_NAME>
```

## 2. 공통 요구사항

* OS:

  * Linux (Ubuntu 20.04 이상 권장) 또는 macOS (버전 명시)
* 공통 패키지:

  * CMake
  * C++17 컴파일러 (g++ / clang++)
  * Docker / Docker Compose (infra-inception용)
* 각 프로젝트별 추가 요구사항은 아래에 정리한다.

---

## 3. 프로젝트별 빌드 및 실행

### 3.1 minishell-cpp17

#### 요구사항

* CMake >= 3.x
* C++17 지원 컴파일러

#### 빌드

```bash
cd minishell-cpp17
cmake -S . -B build
cmake --build build
```

#### 실행

```bash
./build/minishell
```

#### 테스트

```bash
cd minishell-cpp17
ctest --test-dir build
```

---

### 3.2 webserv-cpp17

#### 요구사항

* CMake >= 3.x
* C++17 지원 컴파일러

#### 빌드

```bash
cd webserv-cpp17
cmake -S . -B build
cmake --build build
```

#### 실행 예시

```bash
./build/webserv --config configs/dev.conf
```

#### 테스트

```bash
cd webserv-cpp17
ctest --test-dir build
```

---

### 3.3 infra-inception

#### 요구사항

* Docker
* Docker Compose (또는 호환 도구)

#### 실행

```bash
cd infra-inception
docker compose up -d
```

#### 중지

```bash
docker compose down
```

#### 참고

* 사용 중인 포트, 서비스 목록, 계정/비밀번호 등은 design/infra-inception/ 문서와
  docker-compose.yml 주석을 함께 참고한다.

---

## 4. 버전별 유의사항

* 각 프로젝트의 현재 지원 버전과 특징은 VERSIONING.md를 참조한다.
* 특정 버전 기준으로 클론/빌드하려면 해당 태그를 체크아웃한다.

```bash
git checkout tags/minishell-cpp17-v0.2.0
```

* 새 버전이 추가되어 빌드/실행 방법이 바뀌면:

  * 이 문서를 반드시 업데이트해야 한다.
  * 변경된 요구 패키지/환경 변수를 명확히 기록한다.

````

---

## 4. Consistency checklist (Korean-only outputs)

When an agent finishes work for a given version of a project, it must ensure:

1. **Code**
   - All new/modified files have header comments in Korean.
   - All public functions/methods have Korean comments with:
     - Purpose
     - Inputs/outputs
     - Errors
     - Related design doc
     - Related tests
   - Version tags in comments match the target version (e.g. `v0.2.0`).

2. **Design docs (`design/`)**
   - A new or updated design file exists for the target version:
     - Path: `design/<project>/vX.Y.Z-*.md`
     - Content is written in Korean.
   - The document describes:
     - Purpose and scope
     - External behavior / API contracts
     - Internal architecture
     - Test strategy
     - Migration/compatibility notes (if needed)

3. **CLONE_GUIDE.md**
   - Any changes to build/run/test procedures are reflected in Korean.
   - New dependencies or environment requirements are documented.

4. **VERSIONING.md (not in this file, but referenced)**
   - The version is marked as implemented/completed.
   - Any known limitations or follow-up work are recorded.

---

## 5. Reusing these templates in other projects

- Other repositories can copy this `DOC_TEMPLATES.md` and adjust:
  - Project names
  - Directory structure examples
  - Tooling (e.g. CMake → Meson, or Docker → other infra)
- The core rules should remain:
  - **English for this template file only**
  - **Korean for all comments and documentation**
  - Clear mapping between:
    - Code (with version tags)
    - Design docs
    - Clone guide / setup instructions