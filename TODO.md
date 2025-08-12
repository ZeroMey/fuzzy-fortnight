# TODO (Build Fixes & Conflict Resolution)

> Canonicalize this list as GitHub Issues (one Issue per checkbox). Use this file only as an index.

## 1) Hardware backends (display & touch)
- [ ] Resolve Display implementation
- [ ] Resolve Touch implementation

## 2) Missing sensor headers referenced by `main.cpp`
- [ ] If deferred, wrap with feature macros (e.g., `#if defined(HAVE_MAX30102)` … `#endif`).

## 3) Final verification
- [ ] Update **README** with the chosen hardware stack and any API changes.
