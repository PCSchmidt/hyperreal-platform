# ADR 0001: Engine-independent simulation core

**Status:** Accepted (2026-10-30)
**Context:** Simulation, replay verification, ML training and agent tooling all need to run headless, in CI, and on a small server without a source-built game engine.
**Decision:** Implement simulation and verification as `hr-core`, a C++20 CMake library with Python bindings. The game engine is a presentation client only.
**Consequences:** + headless training, testing and verification; + MCP/agent integration straightforward; + clear architecture story. - Some engine features (physics, math) are unavailable to authoritative code; duplicated math types require conversion at the boundary.
**Alternatives rejected:** Core as an Unreal plugin (couples verification and ML to the engine); Python-only core (too slow, determinism harder to guarantee).
