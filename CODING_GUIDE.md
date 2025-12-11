# CODING_GUIDE.md

This document defines coding rules for all projects in this repository:

- `minishell-cpp17`
- `webserv-cpp17`
- `infra-inception`
- Any future projects added under this repo

It is written in English for AI/tooling, but **all comments and documentation MUST be written in Korean**, as stated below.

---

## 1. Language and standards

- Primary implementation language:
  - `minishell-cpp17`: C++17
  - `webserv-cpp17`: C++17
  - `infra-inception`: Docker, Docker Compose, shell scripts, configuration files (Nginx, DB, etc.)
- C++ standard:
  - Use **C++17** features only.
  - Avoid compiler-specific extensions unless strictly necessary.

---

## 2. Comment and documentation language (MANDATORY)

- **All comments in source code MUST be written in Korean.**
  - File/module headers
  - Function/method comments
  - Class/component descriptions
  - Inline comments
- **All documentation MUST be written in Korean.**
  - `design/` documents
  - `CLONE_GUIDE.md`
  - Project-specific READMEs (except small English terms/identifiers)
- Code identifiers (variables, functions, classes, filenames) must remain in English.
- Protocol/standard names (e.g. `GET`, `POST`, `EPOLLIN`, `/health`) must stay in English, but their explanations should be in Korean.

For the exact comment formats and document templates, follow **`DOC_TEMPLATES.md`**.

---

## 3. Project layout (high-level)

Recommended structure:

```text
minishell-cpp17/
  src/
  include/
  tests/
  CMakeLists.txt

webserv-cpp17/
  src/
  include/
  tests/
  configs/
  CMakeLists.txt

infra-inception/
  docker-compose.yml
  services/
    app/
    db/
    nginx/
    redis/
    ...
```

* Place C++ headers in `include/` and implementation files in `src/`.
* Place test sources in `tests/`.
* Infra-related definitions for `infra-inception` go under `infra-inception/`.

---

## 4. C++ coding style

### 4.1 Naming

* Types (classes, structs, enums): `PascalCase`

  * `CommandExecutor`, `HttpRequest`, `ShellEnvironment`
* Functions/methods: `camelCase`

  * `parseCommandLine`, `runEventLoop`, `acceptNewConnection`
* Variables: `snake_case`

  * `buffer_size`, `client_fd`, `request_queue`
* Constants: `SCREAMING_SNAKE_CASE`

  * `DEFAULT_PORT`, `MAX_CONNECTIONS`

Do **not** use Korean identifiers.

---

### 4.2 Files and includes

* One main class or tightly related group per `.cpp`/`.hpp` pair.
* Use header guards or `#pragma once` in header files.
* For project-local includes:

  * Prefer `#include "module/header.hpp"` form.
* Do not introduce unused includes.

---

### 4.3 Memory and resource management

* Prefer RAII over manual `new`/`delete`.

  * Use `std::unique_ptr`, `std::shared_ptr` where ownership semantics are clear.
* For OS resources (file descriptors, sockets, pipes, processes):

  * Wrap them in RAII classes (e.g. `FdHandle`, `Pipe`, `ChildProcess`).
  * Ensure deterministic release in destructors.
* Do not leak file descriptors or child processes.

---

### 4.4 Error handling

* For C++ code:

  * Prefer explicit error types or `expected`-style return patterns.
  * Avoid throwing exceptions in hot paths (low-level IO, tight loops).
* Typical pattern:

  * Return `bool` / `std::optional<T>` / custom `Result<T>` with error details.
  * Use structured error types (e.g. `ParseError`, `ExecutionError`).
* When logging or reporting errors:

  * Comments explaining the error condition and handling must be in Korean.

---

### 4.5 Concurrency and blocking

* `minishell-cpp17`:

  * Primary concern is process management and signals.
  * Avoid unnecessary threads.

* `webserv-cpp17`:

  * Use a single-threaded event loop initially.
  * Use select/epoll or similar mechanism for multiple connections.
  * Do not introduce multi-threading unless explicitly required by a version spec in `VERSIONING.md`.

---

## 5. Tests

* Each project must have a `tests/` directory.
* Test framework is implementation-defined (e.g. GoogleTest, Catch2, etc.), but:

  * Test code must be clearly separated from production code.
* For each public function or major feature:

  * Add at least basic positive/negative test cases.
* Test names should reflect the scenario in English, but test **descriptions/comments** must be in Korean if present.

Examples:

```cpp
// 올바른 HTTP 요청을 보낼 때 200 OK가 반환되는지 확인한다.
TEST(HttpServerBasic, Returns200OnValidGet) { ... }
```

---

## 6. Infra / Docker / config style (`infra-inception`)

* All Dockerfiles:

  * Use clear stages (`FROM`, `RUN`, `COPY`, etc.).
  * Add **Korean comments** for non-trivial instructions (why, not just what).
* `docker-compose.yml`:

  * Service names in English.
  * Configuration comments in Korean.
* Nginx, DB, Redis config:

  * Keep directives in original syntax.
  * Explain rationale in Korean comments where needed.

Example (Nginx):

```nginx
# 한국 트래픽 기준으로 적당한 타임아웃과 gzip 설정을 적용한다.
keepalive_timeout  65;
gzip               on;
```

---

## 7. Commit and change scope (for agents)

When an AI agent generates code changes, it should:

* Keep each change set focused on **one project and one version** (see `VERSIONING.md`).
* Ensure:

  * Code changes follow this `CODING_GUIDE.md`.
  * Comments and docstrings are written in Korean.
  * If new public APIs are added, corresponding tests and design docs (after tests) are updated.

---

## 8. Cross-reference with other documents

* For **comment formats** and **document templates**:

  * See `DOC_TEMPLATES.md`.
* For **version definitions and goals**:

  * See `VERSIONING.md`.
* For **agent behavior and development loop**:

  * See `AGENTS.md`.
* For **concrete clone/build/run commands**:

  * See `CLONE_GUIDE.md` (when present).

Agents must keep these documents in sync with the rules defined here.
