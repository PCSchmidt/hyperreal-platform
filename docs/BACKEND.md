# Backend Specification (Async Competition and Verification)

**Status:** Draft v1 - design only. Deferred until after the first title's local gates; designed now so the core's contracts support it.

## 1. Goals and non-goals
**Goals:** accept replay submissions, verify them by headless re-simulation, store results, serve seeded leaderboards and daily seeds, compute ratings, and host benchmark results.
**Non-goals:** accounts beyond a display name, payments, chat, matchmaking for live play, personal data.

## 2. Architecture

```
Client ──HTTPS──> API (stateless) ──> Queue ──> Verifier workers (hr-core headless)
                     │                                  │
                     └──────────> Database <────────────┘
                     └──────────> Object storage (replays, models, bench traces)
```

| Component | Choice | Notes |
|---|---|---|
| API | Small stateless HTTP service (Python FastAPI or C++); OpenAPI spec published | Easy to run locally |
| Verifier | Container running `hr-core` headless; one replay per job; wall-clock and memory limits | Deterministic result or typed rejection |
| Queue | Database-backed job table initially | No broker needed at this scale |
| Database | SQLite (local/dev) → PostgreSQL if hosted | Schema below |
| Object store | Local disk / S3-compatible | Replays addressed by SHA-256 |
| Hosting | Local-first; optional low-cost VPS or serverless container later | Costs recorded when chosen |
| Identity | Anonymous token + optional display name; optional OAuth later | No email stored |

## 3. API (v1)
| Endpoint | Purpose |
|---|---|
| `GET /v1/seeds/daily?title=` | Daily seed + generator version |
| `POST /v1/replays` | Upload replay (size-capped); returns job id |
| `GET /v1/jobs/{id}` | Verification status and result |
| `GET /v1/leaderboards/{title}/{seed}` | Verified scores |
| `GET /v1/ratings/{title}/{role}` | Rating tables |
| `POST /v1/bench/results` | Submit benchmark result (signed, validated) |
All payloads conform to `DATA_CONTRACTS.md`. Errors use the shared typed codes.

## 4. Data model (core tables)
`players(id, display_name, created_at)`, `scenarios(hash, title, seed, generator_ver, ruleset_ver)`, `replays(sha256, scenario_hash, player_id, bytes, uploaded_at)`, `results(replay_sha, verified, scores_json, verifier_build, verified_at, reject_code)`, `ratings(player_id, title, role, mu, sigma, updated_at)`, `bench_results(...)`.

## 5. Threat model
| Threat | Mitigation |
|---|---|
| Forged or edited replay | Server re-simulation; any hash divergence rejects |
| Malformed or malicious upload (parser exploits, zip bombs) | Size caps, strict schema-first parsing, fuzzing the replay parser, verifier sandboxed with CPU/memory/time limits and no network |
| Verifier denial of service | Per-token rate limits, job quotas, queue depth limits, timeouts |
| Leaderboard spam | Rate limits, replay uniqueness, optional proof-of-work token |
| Prompt injection via submitted agent traces | Traces are data, rendered as escaped text, never executed or fed to a model without isolation |
| Secret leakage | No secrets in the repo; environment-injected configuration; no client-held admin keys |
| Privacy | No personal data; display name only; deletion on request |
| Supply chain | Pinned dependencies, lockfiles, dependency audit in CI |

## 6. Observability and operations
Structured logs, request IDs, verifier metrics (queue time, run time, rejection reasons), health endpoint, backup of the database and object store, documented restore test.

## 7. Cost and scale assumptions
Portfolio scale: tens of submissions per day. A verification job re-simulates faster than real time (headless), so one small worker suffices. Costs, if hosted, are recorded in `docs/OPERATIONS.md` when a host is chosen.

## 8. Acceptance tests
Tampered replay rejected (100%), legitimate replays accepted (0% false rejects on a corpus), upload fuzzing finds no crashes in a defined run, restore-from-backup succeeds, rate limiting enforced.
