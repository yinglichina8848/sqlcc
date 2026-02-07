#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "Usage: source scripts/sqlcc-agent-config.sh <agent-id>" >&2
  return 1 2>/dev/null || exit 1
fi

agent_id="$1"
case "$agent_id" in
  codex-ide)
    name="SQLCC-AI(Codex-IDE)"
    email="sqlcc+codex-ide@users.noreply.github.com"
    ;;
  *)
    echo "Unknown agent id: $agent_id" >&2
    echo "Please add mapping in scripts/sqlcc-agent-config.sh" >&2
    return 2 2>/dev/null || exit 2
    ;;
 esac

git config user.name "$name"
git config user.email "$email"

echo "Configured git identity:"
git config user.name
git config user.email
