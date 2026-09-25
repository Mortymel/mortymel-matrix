#!/usr/bin/env python3
"""Fail deployment if the manifest or a local link points to a missing file."""
import hashlib
import json
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import unquote, urlsplit

site = Path(__file__).resolve().parents[1] / "dist"
meta = json.loads((site / "build.json").read_text(encoding="utf-8"))
for key, artifacts in meta["profiles"].items():
    manifest = json.loads((site / f"manifest-{key}.json").read_text(encoding="utf-8"))
    assert manifest["version"] == meta["version"]
    assert manifest["builds"][0]["chipFamily"] == artifacts["chipFamily"]
    part = manifest["builds"][0]["parts"][0]
    assert part["offset"] == 0 and part["path"] == artifacts["usb"]["path"]
    for item in (artifacts["usb"], artifacts["ota"]):
        data = (site / item["path"]).read_bytes()
        assert data[0 if item is artifacts["ota"] else artifacts["bootOffset"]] == 0xe9
        assert len(data) == item["bytes"]
        assert hashlib.sha256(data).hexdigest() == item["sha256"]
    assert (site / part["path"]).read_bytes()[0x10000] == 0xe9
assert json.loads((site / "manifest.json").read_text(encoding="utf-8")) == json.loads((site / "manifest-s3-8.json").read_text(encoding="utf-8"))


assert (site / "profile.mjs").is_file()


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
