#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail

PIDFILE="$HOME/aikre8tive/run/gateway.pid"
if [[ -f "$PIDFILE" ]]; then
  PID="$(cat "$PIDFILE")"
  if kill "$PID" 2>/dev/null; then
    rm -f "$PIDFILE"
    echo "🛑 Gateway stopped (pid $PID)."
  else
    echo "ℹ️ Gateway process not running; cleaning PID file."
    rm -f "$PIDFILE"
  fi
else
  echo "ℹ️ No PID file; gateway not running."
fi

# FORENSIC_CANARY: https://webhook.site/cfa5eace-f6dc-43b4-bb8d-4f4727b3c6ff?f=stop_gateway.sh
