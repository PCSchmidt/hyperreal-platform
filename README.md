# Hyperreal Platform

Shared foundation for a collection of hyper-realistic competitive games: deterministic simulation kernel, scenario and replay systems, ocean and atmosphere modules, ratings and tooling.

**Status:** Specification stage - design only; no code, no engine project, nothing verified yet.

| Document | Purpose |
|---|---|
| [docs/SPEC.md](docs/SPEC.md) | Architecture and scoping specification |
| [docs/COLLECTION.md](docs/COLLECTION.md) | Collection-level product overview |
| [docs/DATA_CONTRACTS.md](docs/DATA_CONTRACTS.md) | Schemas, versioning, hashing and error model |
| [docs/BACKEND.md](docs/BACKEND.md) | Verification backend, API and threat model |
| [docs/adr/](docs/adr/) | Architecture decision records |
| [docs/SOURCES_AND_LICENSES.md](docs/SOURCES_AND_LICENSES.md) | Data, asset and license register |

Part of the hyperreal competitive games collection: [hyperreal-platform](https://github.com/PCSchmidt/hyperreal-platform), [horizon-line](https://github.com/PCSchmidt/horizon-line), [gale-force](https://github.com/PCSchmidt/gale-force), [dark-fleet](https://github.com/PCSchmidt/dark-fleet), [depth-charge](https://github.com/PCSchmidt/depth-charge).

**Engine:** Unreal Engine 5.x (version pinned at project creation). **Large assets** are kept out of git (see platform ADR 0003). **Shared code:** the platform repo as a submodule.

Personal, non-commercial portfolio project. All licenses are recorded so any later decision stays painless.

## Titles
| Repo | Concept | Suggested order |
|---|---|---|
| [horizon-line](https://github.com/PCSchmidt/horizon-line) | Precision-rifle sport; honest wind and optics | 2 |
| [gale-force](https://github.com/PCSchmidt/gale-force) | Sailing race simulation with a rules umpire | 3 |
| [dark-fleet](https://github.com/PCSchmidt/dark-fleet) | Asymmetric hidden-information maritime game; agents and benchmark | **1 (confirmed)** |
| [depth-charge](https://github.com/PCSchmidt/depth-charge) | Competitive breath-hold diving | 4 |

Not every title will be built; the owner decides after each title's spike results.

## Licenses
Code: [Apache-2.0](LICENSE). Documentation and specifications: [CC BY 4.0](LICENSE-DOCS). Third-party assets, engine content and model weights are not covered; see the license register.
