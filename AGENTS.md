# AGENTS.md

This document defines how AI coding agents must behave in this repository.

It applies to all projects in this repo, including but not limited to:

- `minishell-cpp17`
- `webserv-cpp17`
- `philosophers-cpp17`
- `infra-inception` (Inception-style infra stack)
- Any future projects added under the same repository

Human maintainers can always override these rules explicitly.  
If human instructions conflict with this file, **follow the human**.

---

## 1. Files you MUST read first

When an agent starts working in this repository, it must read the following files in this order:

1. `AGENTS.md` (this file)
2. `CODING_GUIDE.md`
3. `DOC_TEMPLATES.md`
4. `VERSIONING.md`
5. The relevant project directory (e.g. `minishell-cpp17/`, `webserv-cpp17/`, `philosophers-cpp17/`, `infra-inception/`)
6. If present, `CLONE_GUIDE.md` and any `design/` documents for the target project

Do **not** modify `AGENTS.md` or `DOC_TEMPLATES.md` unless a human explicitly asks you to.

---

## 2. Language policy (IMPORTANT)

- **All comments in source code MUST be written in Korean.**
  - File/module headers, function/method comments, inline comments, docstrings, etc.
- **All documentation MUST be written in Korean.**
  - `design/` documents
  - `CLONE_GUIDE.md`
  - Project-specific `README` sections (except small English terms or identifiers)
- Code identifiers (variable names, function names, classes, filenames) should remain in English.
- Protocol-specific or standard strings (HTTP headers, IRC commands, shell keywords, etc.) must remain in their original language where appropriate, but explanations around them must be written in Korean.

If you generate English comments or documentation by mistake, you must rewrite them in Korean.

---

## 3. Versioning and projects

### 3.1 Versioning source of truth

- All version definitions and high-level goals are specified in `VERSIONING.md`.
- You MUST NOT invent new MAJOR/MINOR versions by yourself.
  - Only use versions that are defined in `VERSIONING.md`, unless a human explicitly instructs you to add a new one.
- PATCH versions (`X.Y.Z` where only `Z` changes) may be used for bugfixes or refactoring, but:
  - You must still record them in `VERSIONING.md` if you introduce a new PATCH version.

### 3.2 Project separation

- Each project has its own section in `VERSIONING.md`, for example:
  - `minishell-cpp17`
  - `webserv-cpp17`
  - `philosophers-cpp17`
  - `infra-inception`
- When working on a task, you must:
  - Identify **exactly one** target project.
  - Identify **exactly one** target version for that project (e.g. `minishell-cpp17 v0.2.0`).
- Do not mix changes for multiple versions or multiple projects in a single development loop unless a human explicitly tells you to.

---

## 4. Standard development loop

For any project/version combination (e.g. `webserv-cpp17 v0.2.0`), you MUST follow this loop:

1. **Select target project and version**
   - Read `VERSIONING.md`.
   - Identify the exact project and version you should work on.
   - Confirm the goals and completion criteria for that version.

2. **Understand current state**
   - Read existing code in the target project directory.
   - Read any existing `design/` documents for previous versions of that project.
   - Read relevant sections in `CLONE_GUIDE.md` (if it already exists).

3. **Plan the changes**
   - Decide what needs to be implemented or modified to reach the target version.
   - Make sure the plan is consistent with:
     - The goals in `VERSIONING.md`
     - The coding rules in `CODING_GUIDE.md`
   - You do **not** write the detailed `design/` document yet; that will be done **after** implementation and tests.

4. **Implement the changes**
   - Modify or add source files under the appropriate project directory.
   - Follow all rules in `CODING_GUIDE.md` (language, style, error handling, directory structure, etc.).
   - For every public function/method/module that you touch or add:
     - Add or update comments in **Korean**, following the patterns described in `DOC_TEMPLATES.md`.
     - Include version tags (e.g. `v0.2.0`) and links to design documents (you can refer to the expected path such as `design/minishell-cpp17/v0.2.0-<topic>.md`).
       - If the design document does not exist yet, you may reference the **intended** path; you will create the document after tests pass.

5. **Add or update tests**
   - If tests already exist:
     - Update them or add new ones to cover the new behavior.
   - If there are no tests yet:
     - Create initial tests according to `CODING_GUIDE.md` and `DOC_TEMPLATES.md`.
   - Tests must reflect the goals for the target version as defined in `VERSIONING.md`.

6. **Run tests**
   - Use the test commands defined in:
     - `CLONE_GUIDE.md` for that project, or
     - the project-specific README / build scripts.
   - If tests fail:
     - **Do not** mark the version as complete.
     - Fix the issues in code and/or tests.
     - Re-run tests.
     - Repeat until all tests pass.

7. **Only after ALL tests pass: update documentation**
   - **Design documents**
     - Create or update a design document under `design/<project>/`.
     - File naming convention (example):
       - `design/minishell-cpp17/v0.2.0-env-and-builtins.md`
       - `design/webserv-cpp17/v0.3.0-http11-core.md`
       - `design/infra-inception/v0.2.0-nginx-redis-stack.md`
       - `design/philosophers-cpp17/v0.2.0-deadlock-free-strategies.md`
     - Use the templates in `DOC_TEMPLATES.md`.
     - Write in **Korean**.
     - Document:
       - Purpose of the version
       - API/behavior changes
       - Internal architecture decisions
       - Test strategy and important edge cases

   - **CLONE_GUIDE.md**
     - If `CLONE_GUIDE.md` does not exist yet:
       - Create it using the template in `DOC_TEMPLATES.md`.
     - If it exists:
       - Update any sections that are affected:
         - Dependencies
         - Build commands
         - Run commands
         - Project-specific notes for the new version
     - All text must be in Korean.

   - **Versioning**
     - Ensure `VERSIONING.md` accurately reflects the current state:
       - The target version is now marked as “implemented” or “completed”.
       - Any notes about known limitations or follow-up work are added.

8. **Completion criteria**

A version (e.g. `webserv-cpp17 v0.2.0`) is considered **complete** only if:

- All tests pass.
- Code contains Korean comments that follow `DOC_TEMPLATES.md`.
- A corresponding design document exists under `design/<project>/` for this version.
- `CLONE_GUIDE.md` is up-to-date for any changes that affect cloning, building, or running.
- `VERSIONING.md` is updated to reflect the version status.

If any of these are missing, you must treat the version as **incomplete** and continue working.

---

## 5. What you MUST NOT do

- Do not delete or disable tests just to make the test run “green”.
- Do not change `AGENTS.md` or `DOC_TEMPLATES.md` unless explicitly instructed by a human.
- Do not introduce new MAJOR or MINOR versions without updating `VERSIONING.md` and getting human approval.
- Do not write comments or documentation in English, except where strictly necessary for:
  - Protocol names
  - Code identifiers
  - Standard technical terms that must appear in English
- Do not mix unrelated changes for multiple versions or projects in a single development loop.

---

## 6. Directory conventions (guideline)

This repository is expected to follow a structure similar to:

```text
AGENTS.md
CODING_GUIDE.md
DOC_TEMPLATES.md
VERSIONING.md
CLONE_GUIDE.md           # created/updated as versions progress

minishell-cpp17/         # project code
webserv-cpp17/           # project code
philosophers-cpp17/      # project code
infra-inception/         # project code

design/
  minishell-cpp17/
    v0.1.0-*.md
    v0.2.0-*.md
    ...
  webserv-cpp17/
    v0.1.0-*.md
    ...
  philosophers-cpp17/
    v0.1.0-*.md
    ...
  infra-inception/
    v0.1.0-*.md
    ...
````

Agents should follow this convention when creating new design documents or projects.

---

## 7. Summary

1. Always read: `AGENTS.md` → `CODING_GUIDE.md` → `DOC_TEMPLATES.md` → `VERSIONING.md`.
2. Pick exactly one project and one version from `VERSIONING.md`.
3. Implement code following `CODING_GUIDE.md`, with **Korean comments**.
4. Add/update tests and run them; only proceed if all tests pass.
5. After tests pass:

   * Write/update a **Korean** design document under `design/<project>/`.
   * Update **Korean** `CLONE_GUIDE.md` if needed.
   * Update `VERSIONING.md` to reflect the completed version.
6. Never bypass tests, never skip documentation, never write comments/docs in English.
