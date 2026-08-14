# Git hooks

This directory holds git hooks that run the project's fast static-analysis
gates locally, so the findings CI would raise on a pull request are caught on
the developer's machine first.

## Enabling

Hooks that ship inside a repository are not active by default. Enable them once
per clone with:

```sh
git config core.hooksPath .githooks
```

This applies to the current clone only; run it on each machine where you work.
To disable, run `git config --unset core.hooksPath`.

## `pre-push`

Before a push, this hook runs, on the first-party (`meos/`, `mobilitydb/`)
C and header files the branch changes:

- **`tools/check_meos_error_returns.py`** — the meos_error return-or-not
  contract that CI enforces.
- **cppcheck** — the same invocation as the CI `Cppcheck` job
  (`.github/workflows/cppcheck.yml`), using the build's real include paths from
  `compile_commands.json`. Findings are scoped to the lines the branch adds,
  so pre-existing issues in untouched code never block the push.

The hook **fails open**: when it cannot analyse (cppcheck is not installed,
there is no `build*/compile_commands.json`, the work tree is unavailable) it
lets the push through and never blocks. It relies on a configure done with
`-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`, whose `compile_commands.json` it
discovers under any `build*/` directory.

In an emergency a push can bypass the hook with `git push --no-verify`.
