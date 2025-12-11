#!/usr/bin/env bash
# minishell-cpp17 v0.4.0 EOF 테스트: Ctrl+D 입력 시 정상 종료를 확인한다.
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "사용법: eof_exit.sh <minishell_binary>" >&2
  exit 1
fi

binary="$1"
python - "$binary" <<'PY'
import subprocess
import sys
import time
import os
import signal

binary = sys.argv[1]
proc = subprocess.Popen([binary], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
try:
    out, err = proc.communicate("", timeout=3)
except subprocess.TimeoutExpired:
    proc.kill()
    proc.wait()
    print("EOF 입력 후 셸이 종료되지 않습니다.", file=sys.stderr)
    sys.exit(1)

if proc.returncode != 0:
    print("EOF 처리 후 종료 코드가 0이 아닙니다.", file=sys.stderr)
    sys.exit(1)
if "오류" in out or "오류" in err:
    print("EOF 처리 중 불필요한 오류가 출력되었습니다.", file=sys.stderr)
    sys.exit(1)
PY
