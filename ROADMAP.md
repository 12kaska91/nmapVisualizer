This roadmap captures short-term fixes, medium-term features, and long-term goals for nmapVisualizer. It reflects the current workspace state (a small SDL3-based UI, a placeholder `Device`, and a simple `utils.hpp` that spawns nmap on Windows).

Goal contract (small):
- Input: nmap XML (stdout), optionally local assets (icons/fonts)
- Output: an interactive visual scene showing hosts and open ports, with ability to save/load and export images
- Error modes: nmap not found, malformed XML, missing assets — app should fail gracefully and log clear errors

Priority overview (next actions):
1. Immediate triage & fixes (low-risk, high-impact)
   - Fix `Device::render()` and resource handling in `src/graphics.hpp`.
   - Improve window/renderer creation and render loop in `src/main.cpp` (set draw color before clearing, check errors, avoid leaks).
   - Make `utils.hpp` cross-platform: wrap `_popen` vs `popen`, and validate/sanitize scan target input.
2. Model & parsing (medium)
   - Implement a small, testable model (Host, Interface, Port, Service) and converter: nmap XML -> model using `libxml++`.
   - Add unit tests for XML parsing using small fixtures.
3. Visualization & interaction (medium)
   - Replace placeholder `Device` with a robust class owning textures/fonts and exposing `update()`/`render()`.
   - Render hosts as nodes, open ports as badges; implement grid and radial layouts.
   - Add selection, hover tooltips, pan/zoom/reset keyboard controls, and basic filtering UI.
4. UX features (lower-medium)
   - Save/load visualizations (JSON) and export PNG screenshots.
   - Spawn nmap in a background worker and parse XML progressively so the UI remains responsive.
5. Polish & delivery (long-term)
   - Performance: batching, layer sorting, optional GPU paths.
   - Packaging for Windows/Linux, CI with builds and tests, documentation.

Immediate tasks (concrete checklist)
- [ ] Fix `src/graphics.hpp`:
  - Correct dest rect construction (width/height order), avoid heap-allocating SDL_FRect.
  - Ensure textures are released on change and load failures are handled.
- [ ] Fix `src/main.cpp`:
  - Call `SDL_SetRenderDrawColor` before `SDL_RenderClear`.
  - Validate window/renderer creation errors and destroy resources on failure.
- [ ] Make `src/utils.hpp` cross-platform and safer:
  - Provide `run_nmap_xml()` that chooses `_popen` on MSVC and `popen` otherwise.
  - Sanitize `targets` (minimal quoting/validation) and return clear errors when nmap is missing.

Design notes & assumptions
- Assume SDL3 (already used) and libxml++ are available in the build environment. If not, fall back or provide CMake optionals.
- For now, assume a synchronous nmap spawn is acceptable for quick testing; convert to a background worker when moving beyond smoke tests.
- Use JSON (nlohmann/json or simple hand-rolled) for save/load; add dependency only when required.

Tests & validation
- Add unit tests for parsing (happy path + malformed XML). Prefer small, fast tests runnable during development.
- After each substantive change run a quick build and smoke-run to ensure the window launches and textures render.

Security
- Never execute arbitrary user input without sanitization. If passing user-provided arguments to nmap, provide an explicit confirmation UI and/or restrict allowed flags.

Next steps (recommended immediate):
1. Open a small PR that implements the fixes under "Immediate tasks" (safe, small edits to `src/graphics.hpp`, `src/main.cpp`, and `src/utils.hpp`) and add one parsing unit test. This will make the repo easier to iterate on.
2. After that PR is green, implement the `model` layer and parsing pipeline.

If you want I can apply the immediate fixes now and run a quick build/smoke test. Say "apply fixes" and I'll proceed.
- Replace the current `graphics.hpp` placeholder with a well-defined `Device` class that:
	- Owns renderer(s) and resources (textures, fonts).
	- Stores render elements (rects, textures, lines) with safe handles.
	- Exposes `update()` / `render()`  lifecycle methods and camera transforms (pan/zoom).
- Make `utils.hpp` cross-platform (wrap `_popen` vs `popen`) and add basic XML parsing tests for small nmap outputs.

- Implement the data model: Host, Interface, Port, Service and a conversion pipeline from nmap XML → model.
- Implement basic visual mapping: render hosts as nodes, open ports as child glyphs, and simple layouts (grid and radial). Add a legend.
- Add interaction: selection, hover tooltips, filter by port/service/state, and keyboard shortcuts for pan/zoom/reset.
- Add simple saving/loading of visualizations (JSON) and an exporter for PNG screenshots.

- Improve rendering performance: batching, layer sorting, and optional GPU-accelerated paths.
- Add texture & font support, better visuals for nodes (icons per OS/service), and animated transitions for layout changes.
- Add asynchronous scanning (spawn nmap worker, stream/parse XML progressively) so UI remains responsive during scans.
- Add cross-platform packaging pipelines (AppImage for Linux, signed installer or portable zip for Windows).

- Add advanced layouts (force-directed, geographic overlays), clustering for large networks, and export to interactive HTML/JSON visualizations.
- Add plugin architecture for importers, exporters, and custom node renderers.
- Provide cloud integration (optional) for storing scan histories and collaborative review.

- Cross-platform: Windows (MSVC/MinGW), Linux (GCC/Clang).
- Secure: never execute arbitrary user input; sanitize nmap output. Escalate permission-related operations explicitly.
- Testable: unit tests for parsing and key rendering logic; CI runs on push/PR.