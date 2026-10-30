# Hyperreal Platform - Architecture and Scoping Specification

**Created:** 2026-10-29
**Status:** Draft v1 - design only; nothing implemented or verified
**Scope:** The shared technical foundation for four candidate titles: [Horizon Line](https://github.com/PCSchmidt/horizon-line), [Gale Force](https://github.com/PCSchmidt/gale-force), [Dark Fleet](https://github.com/PCSchmidt/dark-fleet) and [Depth Charge](https://github.com/PCSchmidt/depth-charge).
**Related program:** Separate from, and not a replacement for, the Godot portfolio in `regional-intelligence-games` (Regional Analysis Desk, Cable Ship).

> **Verification convention.** Statements about Unreal Engine features, plugin maturity, licenses and data-source terms are the author's understanding at drafting time and are **not verified against the installed engine or current license text**. Each is tagged `[VERIFY]` where it gates a decision. Spike ladders (see each title's `docs/SPIKES.md`) convert these into measured facts.

---

## 1. Purpose and principles

The collection is a set of **hyper-realistic, skill-based, competitive** games in which the realism is part of the skill: you win by perceiving and reasoning about a physically credible world (wind, water, light, sensors), not by reflex alone.

| # | Principle | Consequence |
|---|---|---|
| P1 | **Honest simulation.** What the player sees is sampled from the same state the simulation uses. | Wind cues, wave shapes, water clarity and sensor returns are *rendered from* sim fields, never faked separately. |
| P2 | **Deterministic core, cosmetic periphery.** Anything that decides a result runs in a fixed-timestep deterministic module. Chaos, Niagara, cloth and audio are cosmetic unless proven otherwise. | Replays, server re-simulation, ghost racing and anti-cheat all come from one mechanism. |
| P3 | **Ghost/async first, live later.** Competition is built on seeded identical scenarios and verifiable input recordings before any live netcode. | Live 1v1 and teams are layered on without redesign. |
| P4 | **Data-grounded environments.** Real elevation, bathymetry, weather and traffic statistics drive the worlds; hand art is reserved for hero locations. | A single developer can produce credible scale. |
| P5 | **Every result is explainable.** Each scored outcome can be traced to logged inputs and sim state. | Debrief screens, replays and tuning all use the same trace. |
| P6 | **Share only what two titles need.** Platform modules are extracted when a second title has a concrete use. | Avoids a speculative framework. |
| P7 | **Public sources only for physics and sensors.** Ballistics, sensors and maritime tracking use published, public references or clearly labelled invented parameters. | See Section 12. |

**Non-goals:** a general-purpose engine, a live-service backend with monetization, console certification, mobile, user-generated content in v1, third-party mod support.

---

## 2. Engine, toolchain and repository strategy

### 2.1 Engine
- **Unreal Engine 5.x**, pinned to the latest stable release at install time and not upgraded mid-title without a recorded decision. Prior planning notes cite 5.8 as current `[VERIFY]`.
- **Languages:** C++ for simulation, networking, replay, rating and data ingestion; Blueprints for UI wiring, level logic and tunables; Python editor scripting for data import and batch tooling.
- **Renderer baseline:** Nanite + Lumen + Virtual Shadow Maps, with hardware ray tracing as a quality tier. Software Lumen in the editor for responsiveness `[VERIFY on RTX A4500 laptop]`.
- **Large World Coordinates** for kilometre-scale scenes (Horizon Line, Gale Force, Dark Fleet) `[VERIFY engine default]`.
- **IDE:** Visual Studio 2022 (Community) with the "Game development with C++" workload, or JetBrains Rider.

### 2.2 Repositories (public)

| Repo | Contents |
|---|---|
| `hyperreal-platform` | This spec, **`hr-core` standalone C++ library**, Python bindings, shared UE plugin layer, tools, CI, license register, ADRs |
| `horizon-line` | Title code + UE project + `docs/` |
| `gale-force` | Title code + UE project + docs |
| `dark-fleet` | Title code (headless sim, agents, bench) + UE project + docs. **First title.** |
| `depth-charge` | Title code + UE project + docs |

Game repos consume the platform as a **git submodule** at `extern/hyperreal-platform`. The Unreal integration layer is a thin plugin that links the standalone core (Section 3.1). Rationale: independent versioning per title, one place to fix shared bugs, no monorepo of hundreds of GB. A Perforce depot was considered and rejected for now.

**Licensing.** Code is Apache-2.0 (`LICENSE`); docs, specifications and benchmark data are CC BY 4.0 (`LICENSE-DOCS`). Third-party assets, engine content and model weights are **not** covered and are recorded in each repo's `docs/SOURCES_AND_LICENSES.md`.

**Large files policy (public repos).** The repos do **not** use Git LFS for large assets: LFS bandwidth for public clones counts against the owner's quota `[VERIFY current quota]`. Git holds code, specs, small test fixtures and manifests. Large data (DEM tiles, imagery, AIS extracts, trained weights, Unreal content) lives outside git, referenced by manifest entries with SHA-256 hashes (Section 8.3), and is published as release artifacts or external storage when appropriate. Unreal project content is kept in a separate private location unless every asset's license permits redistribution.

### 2.3 Hardware tiers

| Tier | Machine | Use |
|---|---|---|
| Dev | Dell Precision 7770, i9-12950HX, 64 GB RAM, RTX A4500 Laptop (16 GB VRAM) | Authoring, spikes, PIE. Thermally limited; sustained loads throttle. |
| Reference | RTX 3070-class GPU @ 1080p, 16 GB RAM, NVMe | Playability target: 60 FPS minimum for slow titles, 90+ FPS target for scoped views |
| Showcase | High-end desktop / future hardware | Quality tier and marketing captures |

The Dev machine is **not** a minimum spec. Frame-time budgets are set per title and measured in spikes.

---

## 3. Module architecture

### 3.1 Two layers: a standalone core and an Unreal integration layer

**Decision (ADR 0001):** simulation and verification code is an **engine-independent C++20 library (`hr-core`, built with CMake) with Python bindings (pybind11 or nanobind)**. Unreal consumes it through a thin integration plugin and is responsible only for rendering, input and UI.

Why:
- The same binary runs **headless** for CI, server-side replay verification, tournaments and ML training, with no engine install.
- Python bindings give the ML work a Gymnasium-style environment and make the sim an MCP-servable tool backend.
- It removes the dependency on a source-built Unreal engine for verification servers.
- It makes the engineering split visible: reviewable, testable simulation code versus presentation.

Unreal-dependent modules (HRUE*) contain no authoritative game logic.

```
 UE integration layer (rendering, input, UI, audio)    <- HRUE* plugin(s)
 ------------------------------------------------------
 hr-core (C++20, no engine dependency)                 <- CMake, tests, Python bindings
```

### 3.2 Module map

Modules below are **hr-core** libraries unless suffixed UE. Dependency direction is strictly downward.

```
                     Game modules (per title)
  ┌────────────┬────────────┬────────────┬────────────┐
  │HorizonLine │ GaleForce  │ DarkFleet  │DepthCharge │
  └─────┬──────┴─────┬──────┴─────┬──────┴─────┬──────┘
        │            │            │            │
  ┌─────▼────────────▼────────────▼────────────▼──────┐
  │ HRSessions   HRRating   HRTelemetry   HRAgents    │  services
  ├───────────────────────────────────────────────────┤
  │ HROcean   HRAtmosphere   HRGeo   HRSensors  HRFlow │  domain sims/visuals
  ├───────────────────────────────────────────────────┤
  │ HRReplay          HRNet            HRScenario      │  competitive plumbing
  ├───────────────────────────────────────────────────┤
  │ HRCore (deterministic sim kernel, math, RNG, units)│  foundation
  └───────────────────────────────────────────────────┘
```

| Module | Responsibility | Used by |
|---|---|---|
| **HRCore** | Fixed-timestep scheduler, deterministic RNG, unit-typed math, state hashing, serialization, tolerance helpers | all |
| **HRScenario** | Seeded scenario definition (location, conditions, entities), versioned schema, validation | all |
| **HRReplay** | Input-log + seed + checkpoint recording; deterministic playback; desync detection; export to video camera paths | all |
| **HRNet** | Server-authoritative session layer, input transport, state interpolation, snapshot compression, rollback hooks (optional) | all (later) |
| **HRAtmosphere** | ISA atmosphere, humidity, wind field (mean + turbulence + terrain effect), refractive/visibility models, rendering bridge to sky/fog/heat-shimmer | Horizon Line, Gale Force, Dark Fleet |
| **HROcean** | Spectral wave model (CPU deterministic sampler + GPU renderer from the *same* parameters), buoyancy sampling, spray/foam drivers, underwater-surface bridge | Gale Force, Dark Fleet, Depth Charge |
| **HRFlow** | 3D current/flow-field storage and sampling (tidal, rip, wall, cave) | Depth Charge, Gale Force, Dark Fleet |
| **HRGeo** | Geodesy (WGS84/ECEF/ENU), DEM/bathymetry ingestion, tile cache manifest, georeferencing to UE world | all |
| **HRSensors** | Detection probability framework (visual, radar, EO, SAR, RF), line of sight/horizon, sensor tasking | Dark Fleet, Horizon Line (optics model) |
| **HRRating** | Skill rating (TrueSkill-family and Elo/Glicko variants), calibration tooling, season logic | all |
| **HRSessions** | Lobby, matchmaking, leaderboard, daily seeds, account linkage (EOS or alternative) | all (later) |
| **HRTelemetry** | Structured event log, replay-linked analytics, anomaly features | all |
| **HRAgents** | Classical and learned bot framework, RL/imitation training via Python (PyTorch), ONNX export, evaluation harness | all |
| **HRUI (UE)** | Shared HUD/diagnostic widgets, replay scrubber, accessibility options | all |
| **HRPy** | Python bindings, Gymnasium-style environments, headless runner, MCP server scaffolding | all (Dark Fleet first) |
| **HRAgentKit** | Agent harness: tool schemas, role-filtered tool surface, tracing, eval runner, injection test suite | Dark Fleet first |

**Rule:** a module is added to the platform only after a second title demonstrates a concrete need (P6). Initially `HRCore`, `HRScenario`, `HRReplay` and `HRGeo` are built because every title needs them; the domain modules begin inside the first title that needs them and are extracted when a second adopts them (HROcean is the expected first extraction, shared by Gale Force and Dark Fleet).

---

## 4. Deterministic simulation kernel (HRCore)

### 4.1 Requirements
- Same inputs + same seed + same build → **bit-identical** outcome on the authoritative machine.
- Result verification by **server re-simulation of a submitted input log**, not by trusting a client result.
- Tolerance: clients may run the same code for prediction and cosmetics; a mismatch triggers resync, never silently changes a ranked result.

### 4.2 Design decisions

| Topic | Decision | Notes |
|---|---|---|
| Time step | Fixed `dt` per title (Horizon Line projectile 0.5 ms internal; boats 5-10 ms; Dark Fleet campaign 1 s ticks with sub-steps) | Rendering interpolates; sim never reads wall-clock time |
| Numeric type | `double` for state in domain sims; avoid `float` accumulation in decision paths | Float in rendering only |
| Cross-machine determinism | **Guaranteed only for the same binary and CPU architecture family.** Ranked verification runs on a server build identical to the client build. | Cross-compiler/arch bit-equality is explicitly *not* promised; uses `/fp:strict` (MSVC) and disables fast-math in the sim module `[VERIFY via spike]` |
| Transcendentals | Wrap `sin/cos/exp/log/pow` behind `HR::Math`; choose either correctly-rounded implementations or the platform libm pinned to the build | Prevents silent libm differences |
| RNG | Counter-based (e.g. Philox/PCG) with explicit stream IDs per subsystem | Adding a feature must not shift other streams |
| Iteration order | No unordered containers in sim paths; stable entity IDs | |
| Threading | Sim is single-threaded per world, or parallel with a deterministic reduction order | Parallel work (e.g. wave sampling) must have associative-safe reductions |
| Physics engine | **Chaos is cosmetic** (debris, ragdoll, cloth) unless a title opts a subsystem in after a spike | Results never depend on Chaos |
| State hash | Rolling hash of a canonical state serialization every N ticks, stored in replays | Desync detection and anti-cheat |

### 4.3 Units and invariants
Typed units (metres, seconds, kg, Pa, K, radians) at module boundaries; internal SI. Display conversion (MOA/MIL, knots, fathoms, feet) happens only in UI. Every sim module ships invariant tests (energy bounds, no NaN, range clamps).

---

## 5. Scenario and replay

### 5.1 HRScenario
A scenario is a versioned, validated data object:
```
scenario_id, schema_version, title_id
seed (uint64), rng_stream_map
geo: reference origin (lat/lon/alt), extent, terrain/bathymetry dataset IDs + hashes
atmosphere: time/date, sun position source, weather snapshot ID or synthetic parameters
entities: placements, parameters, initial state
rules: scoring rule set ID + version
```
- **Daily/Weekly seeds:** a seed + a generator version deterministically produce a scenario, so all players face the same conditions without shipping each one.
- **Hand-authored scenarios** (hero locations, tutorials) use the same format.
- Validation checks references, ranges, dataset hashes and rule-set compatibility. Cross-version replay compatibility is handled by pinning generator and rules versions in the replay header.

### 5.2 HRReplay
- **Format:** header (build ID, scenario ID/hash, seed, rule version), input stream (timestamped, quantized), periodic checkpoints (full state snapshot every T seconds for scrubbing), rolling state hashes.
- **Uses:** (a) ghost opponents, (b) server verification of ranked scores, (c) spectator/broadcast cameras, (d) portfolio footage, (e) bug reproduction, (f) bot training data (demonstrations).
- **Scrubbing:** jump to a checkpoint then re-simulate forward; backward is checkpoint-based.
- **Size target:** a 10-minute run under a few MB (inputs only) with checkpoints optional/compressed.
- **Failure policy:** a replay whose recorded hashes diverge on re-simulation is flagged, not silently accepted.

---

## 6. Networking (HRNet) - deferred, but designed now

| Layer | Approach |
|---|---|
| Topology | Dedicated server, server-authoritative simulation. Client-hosted is not used for ranked play. |
| Build requirement | Dedicated server targets in Unreal historically require a **source-built engine** `[VERIFY for current version]`; plan a one-time source build and repeat per engine upgrade. |
| Tick rates | Horizon Line 30 Hz (targets/wind) + discrete shot events; Gale Force 20-30 Hz state, 10 Hz to far boats; Dark Fleet 1-4 Hz; Depth Charge 20-30 Hz |
| Client prediction | Own-entity prediction where responsiveness matters (boat steering, diver movement); reconcile to the authoritative state |
| Interest management | Per-title; trivial in small matches |
| Replication framework | Evaluate the engine's replication stack (including newer Iris `[VERIFY maturity]`) against a thin custom snapshot/delta layer in the spike |
| Anti-cheat | (1) server re-simulation of inputs; (2) sanity bounds on inputs per tick; (3) telemetry anomaly detection (HRTelemetry + HRAgents); (4) no client-trusted results. Kernel-level anti-cheat is out of scope. |
| Async competition | Submit input log; server verifies by re-sim; leaderboards for seeded scenarios. This is the first competitive deliverable. |
| Live play | Phase 2: 1v1 then teams, using the same sim on the server |

---

## 7. Rating, matchmaking and seasons (HRRating, HRSessions)

- **Algorithms:** Elo for simple duels; Glicko-2 or TrueSkill-family for variable-participant and team formats. Asymmetric titles (Dark Fleet) keep **per-role ratings**.
- **Calibration tooling:** replay the bot population and recorded matches to estimate rating variance, matchmaking quality and rating inflation/deflation. Publish a calibration write-up per title (a portfolio artifact).
- **Seeded leaderboards:** ranked by score on identical seeds; ties broken by deterministic secondary metric.
- **Backend:** start with a minimal service (stateless HTTP + small database) or Epic Online Services for identity/lobbies `[VERIFY EOS terms and current offering]`. No payments, no personal data beyond a display name and platform account ID.
- **Seasons:** soft reset formula and decay defined per title. Not needed before live play.

---

## 8. Environment and data pipeline

### 8.1 Principle
Real data produces the *large-scale truth*; hero art produces the *hero views*. Pipelines are scripted, reproducible and hash-pinned.

### 8.2 Candidate public sources (all `[VERIFY license/terms]` before inclusion)

| Need | Candidate source | Notes |
|---|---|---|
| Terrain elevation (US, high-res) | USGS 3DEP lidar-derived DEMs; OpenTopography point clouds | Public domain (USGS); excellent for Horizon Line ranges |
| Terrain elevation (global) | Copernicus DEM GLO-30; SRTM; ASTER GDEM | 30 m class; supplement with lidar where possible |
| Imagery | Sentinel-2 (10 m), NAIP (US, ~0.6 m) | Texturing and landcover classification, not photoreal detail |
| Landcover/vegetation | ESA WorldCover, NLCD, OSM | Drives PCG vegetation placement |
| Coastline/features | OpenStreetMap (ODbL), Natural Earth (public domain) | Attribution and share-alike obligations for ODbL |
| Bathymetry | GEBCO, NOAA NCEI bathymetric products | Gale Force, Dark Fleet, Depth Charge |
| Weather (forecast/reanalysis) | NOAA GFS/HRRR, ECMWF open data, ERA5 | Gale Force live/historical conditions; Horizon Line climate priors |
| Waves | NOAA WaveWatch III products; spectral parameters | Spectrum parameters, not raw forecasts |
| Tides/currents | NOAA CO-OPS API; HYCOM | Gale Force, Depth Charge, Dark Fleet |
| Maritime traffic statistics | Marine Cadastre (US AIS), Danish Maritime Authority open AIS | Statistical traffic models; do not redistribute raw data |
| Orbit mechanics | Public TLE/SGP4 libraries | Dark Fleet fictional constellation parameters, not real tasking |
| 3D assets | Fab / Megascans (UE-use license), CC0 libraries, self-captured photogrammetry | License register mandatory |
| Audio | Freesound CC0, self-recorded | |
| Streaming tiles | Cesium for Unreal + Cesium World Terrain | Free tier is personal/non-commercial; Google Photorealistic 3D Tiles are stream-only and not bundleable |

### 8.3 Data manifest
A `data/manifest.json` per repo lists every dataset: id, provider, URL, license, retrieval date, SHA-256, derived-asset recipe, attribution text. CI fails when a derived asset lacks a manifest entry. The manifest feeds the in-game credits screen.

### 8.4 Georeferencing
`HRGeo` fixes one origin per scenario, converts WGS84 ↔ ECEF ↔ local ENU, and places assets in UE's double-precision world. Curvature, refraction and the geoid are modelled where the title needs them (Horizon Line, Dark Fleet), flat-earth ENU otherwise.

---

## 9. Rendering strategy

| Topic | Approach |
|---|---|
| Geometry | Nanite for scanned/hero assets and terrain detail; landscape + PCG for vegetation and rocks |
| Lighting | Lumen GI; sun/sky atmosphere driven by date/time/latitude (HRAtmosphere); optional hardware RT tier |
| Water | Custom spectral ocean (HROcean) because the stock water plugin does not guarantee a deterministic CPU-side sampler `[VERIFY]`; renders from the same spectral parameters |
| Volumetrics | Volumetric fog/clouds with sim-linked density (haze, humidity, underwater scattering) |
| Optics | Title-specific post chains (scope optics, camera housings, goggles/mask, satellite EO/SAR sensor emulation) |
| Characters | MetaHuman or lightweight rigs as needed; characters are secondary in all four titles |
| Upscaling/AA | TSR and NVIDIA DLSS (supported on the RTX A4500) for the performance tier; native AA for screenshot/benchmark tier |
| Budgets | Per-title frame-time budgets (GPU ms) tracked by automated benchmark maps, run in CI-like local scripts |
| Profiling | Unreal Insights, `stat` commands, GPU Visualizer, nightly benchmark capture with regression thresholds |

Visual honesty rule (P1): any visual cue that carries gameplay information has a **sim-sourced driver** and a test proving the cue follows the sim state.

---

## 10. AI and ML (HRAgents)

| Use | Approach |
|---|---|
| Ghost opponents | Replay playback (no ML) - baseline for every title |
| Scripted benchmark bots | Rule-based bots to define difficulty tiers and test balance |
| Learned bots | Imitation learning from recorded human replays, then RL fine-tuning; an external Python trainer (PyTorch) talking to the headless `hr-core` build via HRPy; the engine's Learning Agents plugin is an optional alternative `[VERIFY maturity]` |
| Headless sim | Every title's sim core must run **without rendering** at >> real time for training and server verification |
| Evaluation | Elo/TrueSkill of bots against humans and against each other; behavioral-diversity checks; cheating-resistance checks (bots must use the same inputs as players) |
| Anomaly detection | Telemetry features (input timing entropy, reaction latencies, trajectory smoothness) to flag impossible-skill outliers for human review |
| Compute | Local GPU training on the RTX A4500; avoid cloud spend unless chosen |
| LLM agents (optional, isolated) | **In scope for Dark Fleet first.** Agents never touch the deterministic core directly: they act through the same typed, role-filtered action API as a human, are exposed as an MCP server over the headless sim, run locally by default (open-weight models via a local runtime) with hosted models optional, and their decisions are recorded in the replay as inputs. Games remain fully playable with agents disabled. See `dark-fleet/docs/AGENTS.md`. |
| ML engineering standard | Every learned component has a problem formulation, baseline, metrics, ablations, reproducible training (seeded, config-driven, tracked), exported model, model card and CI evaluation. See each title's `docs/ML.md`. |

---

## 11. Quality, testing and CI

| Layer | Practice |
|---|---|
| Unit tests | Automation framework / Catch2-style tests for HRCore and each sim module: invariants, golden outputs, cross-validation against reference tables |
| Determinism tests | Run N scenarios twice and across thread counts; compare state hashes. Required to pass for every ranked-capable build. |
| Reference validation | Compare each physical model to published reference data (cited per title) within stated tolerances |
| Replay regression | A corpus of recorded replays re-simulated on each build; hash mismatch = failure |
| Performance | Benchmark maps with captured frame-time percentiles; thresholds stored in the repo |
| Data validation | Manifest/hash checks, scenario schema validation |
| Builds | Local scripts (PowerShell) producing packaged Windows builds; GitHub Actions only for lightweight checks (docs lint, manifest check, Python tooling) since full Unreal builds need a large self-hosted runner |
| Playtesting | Owner-tested; record honestly which results are self-test vs independent |

---

## 12. Boundaries, safety and policy

- **Public-source physics.** Ballistics, sensors and maritime tracking use **public physics and published references only**. Where no public value exists, parameters are invented, labelled as tunable design values, and never derived from non-public sources. All work is done on personal accounts and equipment.
- **Real vs fictional.** Real geography; fictional vessels, companies, people and incidents. No implication of wrongdoing by real identifiable parties.
- **Content ratings/themes.** Horizon Line frames shooting as **precision-rifle sport** (paper, steel, clay; no human or animal targets). No gore. Dark Fleet depicts interdiction without graphic violence.
- **Safety framing for Depth Charge.** Breath-hold physiology is simulated for gameplay; the game does not teach unsupervised real-world breath-hold diving and includes an explicit safety notice (see that spec).
- **Licensing.** No commercial intent. All asset and data licenses are recorded anyway so that a later decision remains painless.
- **Privacy.** No telemetry leaves the machine unless the player opts in for a leaderboard; no personal data.

---

## 13. Program roadmap (gated, effort-agnostic)

Gates are evidence-based; calendar dates are deliberately absent.

| Gate | Deliverable | Exit evidence |
|---|---|---|
| G0 | `hr-core` builds with CMake on Windows and Linux CI; unit-test harness; Python bindings import | CI green; `pip install -e .` works; ADRs recorded |
| G1 | HRCore + HRScenario + HRReplay v0 with a toy sim | Determinism test passes; replay re-sim hashes match across 1,000 runs |
| G2 | HRGeo + first environment import (one DEM tile, georeferenced, tested headless) | Terrain matches reference coordinates within stated tolerance |
| G2U | **Unreal toolchain installed; empty UE project links `hr-core`; packaged build runs outside the editor** (deferred until the first title needs visuals) | Packaged Windows build runs outside the editor |
| G3 | First title's core spike ladder complete | Per-title `docs/SPIKES.md` pass criteria met |
| G4 | First title vertical slice (one scenario, ghost competition, replay, leaderboard stub) | Honest owner playtest recorded |
| G5 | Server-verified async leaderboard | A tampered input log is rejected by the verifier |
| G6 | Second title enters; shared modules extracted | Second title reuses at least two platform modules unmodified |
| G7 | Optional live 1v1 | Two-client session passes desync tests |

### Title order (owner-confirmed first title)
1. **Dark Fleet** - **confirmed first.** Highest ML/agent value; headless-first so the simulation, bots, tracker, agent harness and benchmark need no Unreal and little art. Unreal sensor views come last.
2. **Horizon Line** - the visual and optics flagship; learned wind-reading model.
3. **Gale Force** - drives HROcean and the fixed-step multi-agent sim.
4. **Depth Charge** - most novel physiology and underwater rendering.

Later titles are optional and unscheduled; the owner decides which are built after each title's spike results.

---

## 14. Cross-title risks

| Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|
| Unreal feature/maturity assumptions wrong (Iris, Learning Agents, Mover) | Medium | Medium | `[VERIFY]` tags resolved in spikes before dependence |
| Determinism not achievable across machines | Medium | High for ranked play | Restrict ranked verification to identical server build; spike early (G1) |
| Dev laptop thermal/perf limits mask or exaggerate issues | High | Medium | Test on reference tier; record frame-time percentiles not averages |
| Scope sprawl across four titles | High | High | One title at a time; gates; platform extraction only on second use |
| Data licensing mistakes | Medium | Medium | Manifest + attribution gate |
| Binary asset repository growth | High | Medium | No LFS in public repos; data-by-manifest; assets outside git; strict asset budget |
| Hyper-real art volume unattainable by hand | High | Medium | Data-driven world generation, kit-based assets, narrow hero scenes |
| Competitive layer stalls without players | High | Low (portfolio) | Ghosts, bots and replays make single-player competitive |

---

## 15. Open decisions

See also: [DATA_CONTRACTS.md](./DATA_CONTRACTS.md), [BACKEND.md](./BACKEND.md), [adr/](./adr/).

| Decision | Default | Resolve by |
|---|---|---|
| UE version pin | Latest stable at G0 | G0 |
| Replication stack (engine vs thin custom) | Evaluate in G7 prep; async first | G5-G7 |
| Backend (EOS vs minimal service) | Minimal, deferred | G5 |
| Submodule vs engine-plugin packaging | Submodule | G0 |
| Which titles are built | Owner decision after first spike results | G3 |
| VR support | Out of scope; revisit for Depth Charge | After G4 |
