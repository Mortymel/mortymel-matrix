#!/usr/bin/env python3
"""Fail deployment if the manifest or a local link points to a missing file."""
import hashlib
import json
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import unquote, urlsplit

site = Path(__file__).resolve().parents[1] / "dist"
manifest = json.loads((site / "manifest.json").read_text(encoding="utf-8"))
meta = json.loads((site / "build.json").read_text(encoding="utf-8"))
assert manifest["version"] == meta["version"]
assert manifest["builds"][0]["chipFamily"] == "ESP32-S3"
part = manifest["builds"][0]["parts"][0]
assert part["offset"] == 0
assert part["path"] == "firmware/mortymel-matrix-s3-n8.bin"
for key, filename in (("usb", part["path"]), ("ota", "firmware/mortymel-matrix-s3-n8-ota.bin")):
    data = (site / filename).read_bytes()
    assert data[0] == 0xe9
    assert len(data) == meta[key]["bytes"]
    assert hashlib.sha256(data).hexdigest() == meta[key]["sha256"]
assert (site / part["path"]).read_bytes()[0x10000] == 0xe9, "ESP32-S3 app must be at 0x10000"


class Links(HTMLParser):
    def __init__(self):
        super().__init__()
        self.paths = []

    def handle_starttag(self, tag, attrs):
        for key, val in attrs:
            if (tag == "a" and key == "href") or (tag in ("link", "script") and key in ("href", "src")) or (tag == "esp-web-install-button" and key == "manifest"):
                self.paths.append(val)


for page in site.rglob("*.html"):
    links = Links()
    links.feed(page.read_text(encoding="utf-8"))
    for href in links.paths:
        url = urlsplit(href)
        if url.scheme or url.netloc or href.startswith("#") or href.startswith("mailto:"):
            continue
        target = (page.parent / unquote(url.path)).resolve()
        assert target.is_relative_to(site.resolve()), f"Ruta fuera del sitio: {href}"
        if target.is_dir():
            target /= "index.html"
        assert target.is_file(), f"Enlace roto desde {page.relative_to(site)}: {href}"
print("Manifest, imágenes, huellas y enlaces locales correctos")
