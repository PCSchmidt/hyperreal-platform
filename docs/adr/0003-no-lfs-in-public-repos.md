# ADR 0003: No Git LFS for large assets in public repos

**Status:** Accepted (2026-10-30)
**Decision:** Large data and binary assets are not stored in git or Git LFS. They are referenced by hash-pinned manifest entries and distributed as release artifacts or external storage when licenses allow.
**Rationale:** Public-clone bandwidth counts against the owner's LFS quota `[VERIFY]`; most heavy assets cannot be redistributed under their licenses anyway.
