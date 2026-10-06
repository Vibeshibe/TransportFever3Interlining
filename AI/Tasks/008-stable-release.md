# 008 — Validate and promote a stable milestone

- Status: Pending
- Owner: Assistant prepares; user provides gameplay testing
- Dependencies: 006 validated; 007 if preference variation is included

## Objective and output

Promote a demonstrated, documented milestone from `dev` to `main` with clear
compatibility and release scope. Research completion alone is not a stable
gameplay release.

## Acceptance criteria

- [ ] Declare included features, supported game builds, and unresolved limitations.
- [ ] Review code, dependency/tool versions, installation, and rollback instructions.
- [ ] Record applicable task acceptance results and in-game regression evidence.
- [ ] Check save/load and, if applicable, shared-extender consumer compatibility.
- [ ] Confirm no game binaries, credentials, downloaded tools, or private saves
      are included in the release commit or artifacts.
- [ ] Prepare readable release notes, then promote the validated milestone to
      `main` and record its tag/commit and validation evidence.

## Validation and open questions

No release is currently eligible. Keep ongoing work and milestone pushes on
`dev`; define version and artifact format when the implementation path is known.

Related implementation commits: none yet. Planning history: `git log -- AI/Tasks/008-stable-release.md`.
