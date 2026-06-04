How to Contribute
=================
We welcome contributions from the community to make MobilityDB better!

If you want to connect with us, you may always join the gitter chat, or contact us via mailing list. We want you working on things you're excited about. Drop a message and if some one can assist you, will contact you back ASAP.

Here are many ways you can contact us:
*   Gitter chat at [![Gitter](https://badges.gitter.im/MobilityDBProject/MobilityDB.svg)](https://gitter.im/MobilityDBProject/MobilityDB?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)
*   [User mailing list](http://lists.osgeo.org/mailman/listinfo/mobilitydb-users)
*   [Developer mailing list](http://lists.osgeo.org/mailman/listinfo/mobilitydb-dev)

If you haven't yet, Chapter 1 of the [manual](https://docs.mobilitydb.com/MobilityDB/master/) describes how to download and build the sources.

Reporting Issues and Suggesting Features
----------------------------------------
When contributing to this repository, please first search the issues to see if your problem/wish has already been reported. If so, add any extra context you might have found, or at least indicate that you too are having the problem/wish. This will help us prioritize common issues.

If your problem is unreported, create a new [issue](https://github.com/MobilityDB/MobilityDB/issues) for it. This will allow the community to discuss it. When creating an issue, you will be able to choose between multiple templates that we provide to assist in making a clear description of the issue. If you further want to contribute a solution, please go forward and create a pull request.

Pull Request Process
--------------------
You can fork this repository, make your own contribution, and submit a pull request. Please put as the pull request title the issue number that it closes (for example, `[Closes issue #xxx]`). Write a clear log message for your commits. One-line messages are fine for small changes, but bigger changes should have more information.

### Check open PRs before you start

Before you write a line of code — whether you're adding a feature, fixing a bug, or responding to a red CI run on someone else's PR — search the open pull-request queue for prior work in the same area:

```sh
gh pr list --state open --limit 30
gh pr list --state open --search "<feature-or-fixture-name>"
gh search code --repo MobilityDB/MobilityDB "<symbol-or-error-string>"
```

Read each candidate PR's *body*, not just its title — the failure mode you're chasing is often documented as "previous approach (X) caused Y; this PR replaces it with Z." If you find a PR that already addresses your case, comment on it (or on the issue it closes) rather than opening a parallel fix.

This applies just as strongly when investigating a red CI run: the test failure on PR A may already be fixed in PR B that hasn't merged yet. Pushing a parallel fix to PR A is noise; rebasing PR A on PR B (or waiting for B to land) is the right outcome.

Same rule extends across the ecosystem when a change in one repo could land before related changes in a sibling repo (MobilityDB ↔ JMEOS ↔ MobilityDuck ↔ MobilitySpark ↔ PyMEOS ↔ MEOS-API).

### Pre-push checks

Before pushing a commit that opens or updates a pull request, run

```sh
tools/scripts/pre-push-check.sh
```

from the repo root. The script configures the build the way the `coverage: 1` matrix variant of `pgversion.yml` does (`-DCBUFFER=ON -DPOSE=ON -DRGEO=ON -DH3=ON`), builds, and runs the regression test suite. If anything fails, fix it before pushing.

Rationale: the GitHub-hosted CI matrix runs across PostgreSQL 16 / 17 / 18 on Linux, plus macOS and Windows MSYS2. The coverage variant is the most exhaustive Linux build — running it locally catches the failure modes the project has historically tripped over (missing `libh3-dev`, opt-in subsystem regressions, type-catalog gaps). The script's own header documents the gaps it cannot cover (Windows MSYS2 build, macOS build, Coveralls upload); flag those gaps explicitly in the PR description if they apply.

The "CI green before push" convention has two documented exceptions:

* **External infrastructure outages** — e.g. a transient `coveralls.io 502 Bad Gateway` during lcov upload, a quay.io 502 on a Docker base image pull, npm registry timeout. Re-run the failed job once the service recovers; do not change the code in response.
* **Coverage / Codacy thresholds** — `ACTION_REQUIRED` from Codacy on large refactors, where the maintainer has reviewed and accepted the metric drift. Document the override in the PR description.

Anything outside these two exceptions is a real failure and must be resolved before the next push.

Contribution Agreement
----------------------
MobilityDB source code is provided under the [PostgreSQL license](https://github.com/MobilityDB/MobilityDB/blob/master/LICENSE.txt). The documentation is provided under the [Creative Commons Attribution-Share Alike 3.0 License](https://creativecommons.org/licenses/by-sa/3.0/). Any contribution will automatically fall to the same license respectively.

Code of Conduct
---------------
We have a [code of conduct](https://github.com/MobilityDB/MobilityDB/blob/master/code-of-conduct.md), so please follow it in all your interactions with the project.
