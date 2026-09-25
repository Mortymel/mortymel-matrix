#!/usr/bin/env python3
"""Embed the web UI in the firmware; initial installation needs no FS upload."""
from pathlib import Path
root = Path(__file__).resolve().parents[1]
page = (root / 'web/index.html').read_text()
assert ')MORTYMEL_WEB"' not in page
(root / 'include').mkdir(exist_ok=True)
(root / 'include/web_ui.h').write_text(
    '#pragma once\nstatic const char WEB_UI[] PROGMEM = R"MORTYMEL_WEB(\n'
    + page + '\n)MORTYMEL_WEB";\n'
)
