# VERSIONING.md

This document defines the versioning scheme and roadmap for each project in this repository.

All version numbers follow:

- `MAJOR.MINOR.PATCH` (e.g. `1.0.0`)

---

## 1. Global versioning rules

- `0.x.y`
  - Development / experimental phase.
  - APIs and internal structure may change.
- `1.0.0`
  - First portfolio-ready release.
  - Features, tests, and documentation (in Korean) are stable enough to be shared publicly.
- `1.x.y`
  - Feature additions and improvements that remain backward-compatible with `1.0.0`.
- `PATCH` (`X.Y.Z` where only `Z` changes)
  - Bug fixes
  - Minor refactors that do not change external behavior
  - Documentation/test-only updates

**Important:**

- Design documents and comments for each version MUST be written in Korean.
- AI agents MUST NOT invent new MAJOR/MINOR versions that are not listed here, unless explicitly instructed by a human.
- New PATCH versions may be introduced for bugfixes, but they must be recorded here.

---

## 2. minishell-cpp17

A C++17-based reimplementation of a minimal POSIX-style shell.

### v0.1.0 – Minimal command execution

**Goal**

- Execute a single simple command with arguments.
- Handle basic exit codes.

**Scope**

- Read one line of input.
- Split by whitespace into program + arguments.
- Execute via `fork` + `execve` (or equivalent).
- Print exit status when the process terminates.

**Completion criteria**

- Basic tests exist and pass (single command execution).
- Korean comments added for core modules/functions.
- Design doc: `design/minishell-cpp17/v0.1.0-minimal-shell.md` written in Korean.
- `CLONE_GUIDE.md` updated with basic build/run instructions for `minishell-cpp17`.
- **Status:** 구현 완료.

---

### v0.2.0 – Environment variables and basic builtins

**Goal**

- Support environment variable expansion.
- Implement minimal builtins: `cd`, `exit`, `env` (scope can be adjusted if needed).

**Scope**

- Parser/expander:
  - Expand `$VAR` from the environment.
- Builtins:
  - `cd <path>`
  - `exit [status]`
  - `env` (print environment variables)
- Refactor input handling if necessary.

**Completion criteria**

- Tests for:
  - `$VAR` expansion
  - Builtins behavior
- Korean comments updated with version tags and design doc links.
- Design doc: `design/minishell-cpp17/v0.2.0-env-and-builtins.md`.
- `CLONE_GUIDE.md` updated if usage changes.
- **Status:** 구현 완료.

---

### v0.3.0 – Pipes and redirections

**Goal**

- Support `|`, `<`, `>`, `>>` in command lines.

**Scope**

- Parser:
  - Build an AST that represents pipelines and redirections.
- Executor:
  - Create and manage pipes using RAII wrappers.
  - Handle input/output redirection.

**Completion criteria**

- Tests covering:
  - Simple pipelines
  - Multiple pipes
  - Input/output redirections
- Design doc: `design/minishell-cpp17/v0.3.0-pipelines-and-redirections.md`.
- All core execution paths commented in Korean.

- **Status:** 구현 완료.

---

### v0.4.0 – Signals and error handling

**Goal**

- Reasonable handling of `Ctrl+C`, `Ctrl+D` and error reporting.

**Scope**

- Signal handling:
  - Interrupt current command with `Ctrl+C` but keep shell alive.
  - Properly handle EOF (`Ctrl+D`).
- Error reporting:
  - Structured error types (e.g. `ParseError`, `ExecutionError`).

**Completion criteria**

- Tests for signal behavior and error conditions.
- Design doc: `design/minishell-cpp17/v0.4.0-signals-and-errors.md`.
- `CLONE_GUIDE.md` updated if any run instructions change.
- **Status:** 구현 완료.

---

### v1.0.0 – Portfolio-ready minishell

**Goal**

- Deliver a clean, documented, and tested minishell suitable for portfolio use.

**Scope**

- Stabilize features from v0.1.0–v0.4.0.
- Improve comments and design documents.
- Add a short user-facing README section in Korean explaining how to use it.

**Completion criteria**

- All previous version criteria met.
- Design doc: `design/minishell-cpp17/v1.0.0-overview.md` summarizing final architecture.
- `VERSIONING.md` marks v1.0.0 as “portfolio-ready”.
- **Status:** 구현 완료.

---

## 3. webserv-cpp17

A C++17 HTTP server inspired by basic `webserv`/Nginx-like behavior.

### v0.1.0 – Single-connection blocking HTTP server

**Goal**

- Serve static files over HTTP to a single client at a time.

**Scope**

- Listen on a configured port.
- Accept one client.
- Read a simple HTTP/1.0 request (`GET`).
- Return a static file or basic error responses.

**Completion criteria**

- Tests for:
  - Valid GET request for existing file.
  - 404 for missing file.
- Design doc: `design/webserv-cpp17/v0.1.0-basic-http-server.md`.
- Korean comments in core server loop.
- **Status:** 구현 완료.

---

### v0.2.0 – Multi-connection event loop

**Goal**

- Support multiple simultaneous client connections.

**Scope**

- Introduce a non-blocking event loop using `select`, `poll`, or `epoll`.
- Manage connection states and timeouts.

**Completion criteria**

- Tests for:
  - Multiple concurrent connections.
  - Connection timeout behavior.
- Design doc: `design/webserv-cpp17/v0.2.0-multi-connection-loop.md`.
- `CLONE_GUIDE.md` updated if run options/configs change.
- **Status:** 구현 완료.

---

### v0.3.0 – HTTP/1.1 core features

**Goal**

- Handle essential HTTP/1.1 behavior.

**Scope**

- Host header handling.
- Basic support for persistent connections (keep-alive).
- Request/response parsing improvements (headers, body).

**Completion criteria**

- Tests for:
  - Host handling
  - Keep-alive behavior
- Design doc: `design/webserv-cpp17/v0.3.0-http11-core.md`.

- **Status:** 구현 완료.

---

### v0.4.0 – Dynamic responses (CGI or handler routing)

**Goal**

- Support simple dynamic behavior.

**Scope**

- Implement either:
  - CGI execution, or
  - Application-level handler routing (e.g. function handlers mapped to paths).
- Add `/health` and optionally `/metrics` endpoints.

**Completion criteria**

- Tests for dynamic endpoints.
- Design doc: `design/webserv-cpp17/v0.4.0-dynamic-handlers.md`.
- Example configuration included under `webserv-cpp17/configs/`.
- **Status:** 구현 완료.

---

### v1.0.0 – Portfolio-ready web server

**Goal**

- Provide a clean, documented HTTP server suitable for portfolio demonstration.

**Scope**

- Stability and documentation polish.
- Korean README section explaining architecture and usage.
- Optional: example Nginx reverse proxy config for integration.

**Completion criteria**

- All previous version criteria met.
- Design doc: `design/webserv-cpp17/v1.0.0-overview.md`.
- `VERSIONING.md` marks v1.0.0 as “portfolio-ready”.
- **Status:** 구현 완료.

---

## 4. philosophers-cpp17

A C++17-based concurrency lab focusing on the classic dining philosophers problem.

### v0.1.0 – Naive dining philosophers (deadlock demo)

**Goal**

- Implement the simplest version of the dining philosophers problem to demonstrate **deadlock** clearly.

**Scope**

- Implement N philosophers and N forks arranged in a circle.
- Use C++17 threads and mutexes (`std::thread`, `std::mutex`).
- Each philosopher repeatedly:
  - Thinks
  - Tries to pick up the left fork, then the right fork
  - Eats for a short time
- The naive locking strategy should allow a deadlock state to appear under reasonable conditions.
- Log philosopher states (thinking, hungry, eating) and highlight when the system appears to be stuck.

**Completion criteria**

- Builds as a simple CLI program.
- Provides an easy way (e.g. CLI flags or default config) to reproduce deadlock.
- Korean comments explain:
  - How the locking strategy works.
  - Why deadlock can occur in this setup.
- Design doc: `design/philosophers-cpp17/v0.1.0-naive-deadlock.md` (Korean).
- `CLONE_GUIDE.md` contains build/run instructions for `philosophers-cpp17`.
- **Status:** 구현 완료.

---

### v0.2.0 – Deadlock-free strategies

**Goal**

- Implement at least two **deadlock-free** strategies and compare them against the naive version.

**Scope**

- Extend v0.1.0 to support multiple strategies, for example:
  - Ordered locking: always lock the lower-indexed fork first, then the higher.
  - Waiter/arbiter: a central coordinator that decides who may pick up forks.
- Add CLI options such as:
  - `--strategy naive|ordered|waiter`
  - `--philosophers N`
- During execution, log:
  - Whether deadlock is detected.
  - Basic per-philosopher statistics (e.g. number of meals).

**Completion criteria**

- Naive strategy can still deadlock; other strategies must not deadlock under the same conditions.
- Korean comments in core strategy code explain:
  - The intuition behind each strategy.
  - Why it prevents deadlock.
- Design doc: `design/philosophers-cpp17/v0.2.0-deadlock-free-strategies.md` (Korean).
- `CLONE_GUIDE.md` updated with examples showing how to run each strategy.

---

### v0.3.0 – Starvation and fairness metrics

**Goal**

- Go beyond deadlock and provide simple metrics for **starvation** and **fairness**.

**Scope**

- For each strategy, collect statistics such as:
  - Meals per philosopher.
  - Maximum continuous waiting time per philosopher.
  - Simple distribution/variance measures.
- Add CLI options to control:
  - Total run time.
  - Random seed (if any randomness is used).
- Print a summary report at the end of execution showing:
  - Per-philosopher meal counts.
  - Indicators of possible starvation or unfairness.

**Completion criteria**

- The program can run the same scenario with different strategies and print comparable statistics.
- Design doc: `design/philosophers-cpp17/v0.3.0-starvation-and-fairness.md` (Korean) explains:
  - Which metrics are used.
  - How to interpret them.
- Korean comments connect these metrics to real-world server/concurrency issues.

---

### v1.0.0 – Concurrency lab (portfolio-ready)

**Goal**

- Provide a small, well-documented “concurrency lab” suitable for explaining deadlock, starvation, and fairness in interviews or portfolio discussions.

**Scope**

- Consolidate previous versions into a clean structure with:
  - Clear separation between strategies, simulation control, and reporting.
  - A simple CLI interface for configuring runs.
- Ensure the code is readable and well-commented in Korean.
- Provide high-level overview documentation for the entire project.

**Completion criteria**

- Design doc: `design/philosophers-cpp17/v1.0.0-overview.md` (Korean) summarizing:
  - Overall architecture.
  - Strategies implemented.
  - How to use the tool as a concurrency teaching aid.
- `CLONE_GUIDE.md` includes:
  - How to build and run philosophers-cpp17.
  - Example commands for comparing strategies.
- `VERSIONING.md` marks `philosophers-cpp17 v1.0.0` as portfolio-ready.

---

## 5. infra-inception

An Inception-style infrastructure stack, tuned for a typical Korean web service scenario.

### v0.1.0 – App + DB stack

**Goal**

- Run an application container and a database container together.

**Scope**

- `docker-compose.yml` with:
  - `app` service (can be a simple demo app initially).
  - `db` service (MariaDB/MySQL).
- Database initialization script (basic schema).

**Completion criteria**

- Basic `docker compose up/down` works.
- Design doc: `design/infra-inception/v0.1.0-app-db-stack.md`.
- `CLONE_GUIDE.md` contains clone/run instructions for `infra-inception`.
- **Status:** 구현 완료.

---

### v0.2.0 – Nginx reverse proxy + Redis

**Goal**

- Add a reverse proxy and cache layer.

**Scope**

- Nginx:
  - Fronts the `app` service.
  - Handles basic routing and static file proxying if needed.
- Redis:
  - Used as cache or session store.

**Completion criteria**

- Design doc: `design/infra-inception/v0.2.0-nginx-redis-stack.md`.
- `CLONE_GUIDE.md` updated for new services and ports.
- **Status:** 구현 완료.

---

### v0.3.0 – “Korean web” tuning

**Goal**

- Apply realistic settings for Korean web environment.

**Scope**

- Timezone:
  - All containers set to `Asia/Seoul`.
- DB:
  - Default charset `utf8mb4`, collation `utf8mb4_unicode_ci`.
- Nginx:
  - Reasonable timeout and `gzip` settings.
  - Access log format including request/response times.

**Completion criteria**

- Design doc: `design/infra-inception/v0.3.0-kor-tuning.md`.
- Config files documented with Korean comments.

- **Status:** 구현 완료.

---

### v0.4.0 – Basic monitoring stack

**Goal**

- Add minimal observability.

**Scope**

- Add monitoring tools (e.g. Prometheus + Grafana, or similar).
- Provide a simple dashboard for HTTP/DB metrics.

**Completion criteria**

- Design doc: `design/infra-inception/v0.4.0-monitoring-stack.md`.
- `CLONE_GUIDE.md` updated with monitoring usage instructions.
- **Status:** 구현 완료.

---

### v1.0.0 – Portfolio-ready infra stack

**Goal**

- Provide a full stack example suitable for portfolio and discussion.

**Scope**

- Polish configuration.
- Final Korean documentation describing architecture and design decisions.
- Ensure reasonable defaults and clean teardown.

**Completion criteria**

- All previous version criteria met.
- Design doc: `design/infra-inception/v1.0.0-overview.md`.
- `VERSIONING.md` marks v1.0.0 as “portfolio-ready”.
- **Status:** 구현 완료.
```