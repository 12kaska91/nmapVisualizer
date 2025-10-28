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