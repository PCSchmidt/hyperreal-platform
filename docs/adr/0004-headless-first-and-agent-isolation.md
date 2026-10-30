# ADR 0004: Headless-first delivery and agent isolation

**Status:** Accepted (2026-10-30)
**Decision:** The first title (Dark Fleet) is delivered headless-first: simulation, role filtering, bots, tracker, MCP server, agent harness and benchmark before any Unreal work. LLM agents act only through the typed, role-filtered action API, never on core state, and their decisions are recorded as replay inputs. Every title remains playable with agents disabled.
**Consequences:** + ML/agent work proceeds without art dependency; + replayable and testable agent runs. - Visual payoff arrives late; mitigated by a simple 2D map front end early.
