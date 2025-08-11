# TODO (Build Fixes & Conflict Resolution)

> Canonicalize this list as GitHub Issues (one Issue per checkbox). Use this file only as an index.

## 1) Hardware backends (display & touch)
- [x] Decide display driver: **ILI9341** or **ST7789**.
- [ ] Decide touch backend: **XPT2046_Touchscreen** or **Arduino_TouchBus** family.
- [ ] Add a unified `hardware.h`:
  - [ ] Declare a single `DisplayT` typedef and `extern DisplayT tft;`
  - [ ] Declare a single touch symbol (e.g., `extern XPT2046_Touchscreen ts;`)
- [ ] Refactor all modules to include `hardware.h`; remove conflicting `extern` declarations.
- [ ] Ensure exactly **one** definition of `tft` and the touch symbol exists.
- [x] Build check: no type mismatches across translation units.

## 2) Duplicate functions → single canonical implementations
- [x] Keep `initDisplay()` in **display_manager.cpp** and declare in **display_manager.h**.
- [x] Keep `initTouchscreen()` and `handleTouchInput()` in **touchscreen.cpp** and declare in **touchscreen.h**.
- [x] Remove/rename duplicates in **main.cpp** and **watch_page.cpp**.
- [ ] Mark page‑local helpers `static` (or wrap in an anonymous namespace).
- [x] Build check: no multiple‑definition link errors for these symbols.

## 3) Global UI symbol collisions (LVGL labels)
- [ ] Make page labels file‑local or uniquely named:
  - [ ] `time_label` in **screensaver_page.cpp** and **watch_page.cpp** → `static` or rename (e.g., `screensaver_time_label`, `watch_time_label`).
  - [ ] `status_label` in **health_page.cpp**, **telemetry_page.cpp**, **screensaver_page.cpp** → `static` or unique names.
- [ ] Build check: no duplicate globals across translation units.

## 4) `utils` API mismatch
- [ ] Decide API for `formatTemperature`:
  - Option A (string): change header to `const char* formatTemperature(float tempCelsius);`
    - [ ] Ensure returned storage is valid (static buffer or caller‑owned); or switch to `std::string`/`String` consistently.
  - Option B (numeric): implement `float formatTemperature(float tempCelsius);` and update call sites.
- [ ] Update header/source to match; fix all call sites.
- [ ] Add a small unit/compile test for the chosen API.

## 5) Missing headers referenced by `main.cpp`
- [x] Provide or guard/remove includes for:
  - [x] `max30102_sensor.h`
  - [x] `mlx90614-dss_sensor.h`
  - [x] `temt6000_sensor.h`
  - [x] `dht22_sensor.h`
  - [x] `QMI8658.h`
  - [x] `SEN0502.h`
- [ ] If deferred, wrap with feature macros (e.g., `#if defined(HAVE_MAX30102)` … `#endif`).

## 6) Declared but not defined
- [ ] Implement `void activateScreensaver();` (declared in **screensaver_page.h**) or remove the declaration.

## 7) Internal helper linkage hygiene
- [ ] In **display_manager.cpp**, make TU‑local helpers `static`/anonymous namespace:
  - [ ] `my_disp_flush`
  - [ ] `loadPageWithFade`
  - [ ] `loadPageWithSlide`
  - [ ] `backToPreviousPage`
  - [ ] `handlePageTransition`
  - [ ] `refreshDisplay`

## 8) Final verification
- [x] Full rebuild: no unresolved externals or ODR violations.
- [x] Runtime smoke test: display init, touch input, page transitions, and LVGL labels behave as expected.
- [ ] Update **README** with the chosen hardware stack and any API changes.
