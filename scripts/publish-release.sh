#!/usr/bin/env bash
set -euo pipefail
version="$(tr -d '\r\n' < VERSION)"
[[ "$version" =~ ^loxone-v[0-9]+\.[0-9]+\.[0-9]+$ ]]
notes="docs/releases/$version.md"
test -s "$notes"
if gh release view "$version" --json isDraft,targetCommitish > /tmp/release-state.json 2>/dev/null; then
  if python3 -c 'import json,sys; sys.exit(0 if not json.load(open("/tmp/release-state.json"))["isDraft"] else 1)'; then
    echo "Published release $version already exists; preserving its assets."
    exit 0
  fi
  python3 -c 'import json,os; assert json.load(open("/tmp/release-state.json"))["targetCommitish"] == os.environ["GITHUB_SHA"], "Existing draft points to a different commit"'
else
  gh release create "$version" --draft --target "$GITHUB_SHA" --title "HomeKey Loxone ${version#loxone-}" --notes-file "$notes"
fi
gh release upload "$version" release-dist/* --clobber
gh release edit "$version" --draft=false --latest
