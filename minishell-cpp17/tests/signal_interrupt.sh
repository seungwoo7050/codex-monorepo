#!/usr/bin/env bash
# minishell-cpp17 v0.4.0 시그널 테스트: Ctrl+C 처리 후 셸이 살아있는지 확인한다.
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "사용법: signal_interrupt.sh <minishell_binary>" >&2
  exit 1
fi

binary="$1"
python - "$binary" <<'PY'
import os
import signal
import subprocess
import sys
import time

binary = sys.argv[1]
proc = subprocess.Popen([binary], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
try:
    time.sleep(0.2)
    proc.stdin.write("sleep 5\n")
    proc.stdin.flush()
    time.sleep(0.5)
    os.kill(proc.pid, signal.SIGINT)
    time.sleep(0.2)
    proc.stdin.write("echo alive\nexit\n")
    proc.stdin.flush()
    out, err = proc.communicate(timeout=10)
except Exception as exc:
    proc.kill()
    proc.wait()
    raise

if "alive" not in out:
    print("인터럽트 이후 명령을 처리하지 못했습니다.", file=sys.stderr)
    sys.exit(1)

if proc.returncode != 0:
    print("시그널 처리 후 종료 코드가 0이 아닙니다.", file=sys.stderr)
    sys.exit(1)
PY
