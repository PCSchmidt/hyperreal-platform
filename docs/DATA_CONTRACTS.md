# Data Contracts

**Status:** Draft v1 - design only. Schemas are normative for the platform once implemented; field-level details are refined in G1 and recorded as versioned JSON Schema files under `schemas/`.

## 1. Principles
1. **Every persisted or transmitted artifact has a `schema_version` (semver) and a `content_hash`.**
2. **Forward/backward policy:** minor versions add optional fields only; a major bump requires a migration function and a migration test with archived fixtures.
3. **Canonical serialization** for anything hashed: UTF-8 JSON with sorted keys, no insignificant whitespace, numbers in a fixed format (integers as integers; doubles as shortest round-trip decimal), or a documented binary encoding. Hashes are SHA-256.
4. **Units are SI** in files; unit names are in field names where ambiguity is possible (`speed_mps`, `lat_deg`).
5. **Time is UTC**, stored as integer milliseconds since the Unix epoch plus an explicit simulation tick.
6. **No hidden state in artifacts shown to a restricted role.** Role-filtered artifacts are distinct schemas (see `ObservationV1`).

## 2. Core artifacts

### 2.1 Scenario (`scenario.v1.json`)
```
{
  "schema_version": "1.0.0",
  "scenario_id": "df-hormuz-0001",
  "title_id": "dark-fleet",
  "generator": {"name": "...", "version": "..."},
  "seed": "uint64 as decimal string",
  "rng_streams": {"traffic": 1, "weather": 2, "sensors": 3, "events": 4},
  "geo": {"origin_lat_deg": 0.0, "origin_lon_deg": 0.0, "extent_nm": 0,
           "datasets": [{"id": "...", "sha256": "..."}]},
  "time": {"start_utc_ms": 0, "tick_s": 60, "duration_ticks": 0},
  "rules": {"ruleset_id": "...", "ruleset_version": "..."},
  "entities": [ ... title-specific, validated by a title schema ... ],
  "content_hash": "sha256-hex"
}
```
Validation: JSON Schema + semantic checks (references resolve, ranges valid, dataset hashes match manifest).

### 2.2 Action log / replay (`replay.v1`)
Binary container (chunked) with a JSON header:
```
header: {schema_version, build_id, scenario_hash, seed, ruleset, role_map, tick_s, n_ticks}
chunks: INPUTS (tick, actor_id, action_type, payload; varint-encoded),
        CHECKPOINTS (tick, compressed canonical state, state_hash),
        HASHES (tick, rolling_state_hash),
        AGENT_TRACE (optional: tool calls and model outputs that produced inputs),
        META (client_info, timing)
footer: {content_hash}
```
Rules: replaying INPUTS from the scenario must reproduce every HASHES entry. AGENT_TRACE is informational; inputs are authoritative, which makes LLM-in-the-loop runs replayable without re-querying a model.

### 2.3 Observation (`observation.v1`) - role-filtered view
Only the fields a role may know. Every observation has `role`, `tick`, `observed_entities[]` (each with source sensor, uncertainty ellipse, confidence, optional identity claims) and `events[]`. A fuzz test asserts that no field of the truth state can appear here except through a sensor model.

### 2.4 Action (`action.v1`)
Typed, validated, role-scoped: `{role, tick, action_type, params}`. The action catalog is a versioned enum per title. Invalid actions are rejected with a typed error code, never silently coerced.

### 2.5 Result (`result.v1`)
`{scenario_hash, replay_hash, scores{role: value}, events[], verified: bool, verifier_build_id}`. Derived purely from the replay.

### 2.6 Dataset manifest (`data/manifest.json`)
```
[{"id","provider","url","license","retrieved_utc","sha256","bytes","derived_by","attribution"}]
```
CI fails if a derived asset lacks an entry or a hash mismatches.

### 2.7 Model card (`model-card.v1.md` + `model.json`)
Required for every shipped learned component: intended use, training data lineage (scenario seeds, generator versions), algorithm and hyperparameters, metrics with confidence intervals, baseline comparison, limitations, license, ONNX hash, training commit.

### 2.8 Benchmark result (`bench-result.v1.json`)
`{bench_id, bench_version, agent_id, model_id, scenario_seeds[], per_scenario_metrics[], aggregate{metric: {mean, ci95}}, cost{tokens_in, tokens_out, usd?}, latency_ms{p50,p95}, trace_ref}`.

## 3. Versioning and compatibility policy
- Replays record `build_id` and rule versions; the verifier refuses mismatched rule versions unless a migration exists.
- A **compatibility corpus** (archived scenarios and replays) is replayed on every build; any hash mismatch fails CI.
- Save data and client settings follow the same version/migration rules; corrupt or incompatible files are surfaced, never silently overwritten.

## 4. Error model
Typed error codes (`E_SCHEMA`, `E_VERSION`, `E_HASH`, `E_RULE_VIOLATION`, `E_ROLE_FORBIDDEN`, `E_DESYNC`, `E_LIMIT`) shared across the core, API and agent tool surface so failures are explainable and testable.
