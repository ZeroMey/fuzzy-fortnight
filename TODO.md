# TODO (Build Fixes & Conflict Resolution)

> Canonicalize this list as GitHub Issues (one Issue per checkbox). Use this file only as an index.

## 1) Hardware backends (display & touch)
- [ ] Resolve Display implementation
- [ ] Resolve Touch implementation

## 2) Global UI symbol collisions (LVGL labels)
- [ ] Make page labels file‑local or uniquely named:
  - [ ] `time_label` in **screensaver_page.cpp** and **watch_page.cpp** → `static` or rename (e.g., `screensaver_time_label`, `watch_time_label`).
  - [ ] `status_label` in **health_page.cpp**, **telemetry_page.cpp**, **screensaver_page.cpp** → `static` or unique names.
- [ ] Build check: no duplicate globals across translation units.

## 3) `utils` API mismatch
- [ ] Decide API for `formatTemperature`:
  - Option A (string): change header to `const char* formatTemperature(float tempCelsius);`
    - [ ] Ensure returned storage is valid (static buffer or caller‑owned); or switch to `std::string`/`String` consistently.
  - Option B (numeric): implement `float formatTemperature(float tempCelsius);` and update call sites.
- [ ] Update header/source to match; fix all call sites.
- [ ] Add a small unit/compile test for the chosen API.

## 4) Missing sensor headers referenced by `main.cpp`
- [ ] If deferred, wrap with feature macros (e.g., `#if defined(HAVE_MAX30102)` … `#endif`).

## 5) Final verification
- [ ] Update **README** with the chosen hardware stack and any API changes.
