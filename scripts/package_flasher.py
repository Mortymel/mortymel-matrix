#!/usr/bin/env python3
"""Build ESP Web Tools manifests from the three ESP32-S3 QD profiles."""
import hashlib
import html
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

import markdown

ROOT = Path(__file__).resolve().parents[1]
APP_SLOTS = {"4": 0x1A0000, "8": 0x330000, "16": 0x640000}
PROFILES = {"4": ("s3-4mb", "mortymel-matrix-s3-4mb"),
            "8": ("esp32-s3", "mortymel-matrix-s3-n8"),
            "16": ("s3-16mb", "mortymel-matrix-s3-16mb")}
SITE = ROOT / "dist"
VERSION = (ROOT / "VERSION").read_text().strip()
GUIDES = {
    "INSTALACION": ("Instalación y recuperación", "Primer arranque, Wi-Fi, panel y actualizaciones."),
    "CABLEADO": ("Conexión física HUB75", "Señales, GPIO, alimentación y comprobaciones antes de encender."),
    "MQTT": ("Home Assistant y MQTT", "Discovery, estados, comandos y automatizaciones."),
    "API": ("API web", "Rutas para escenas, archivos, ajustes y OTA."),
    "ARQUITECTURA": ("Arquitectura", "Cómo funciona el firmware y cuáles son sus límites."),
    "DESARROLLO": ("Desarrollo", "Compilación, empaquetado y verificaciones."),
    "ROADMAP": ("Hoja de ruta", "Compatibilidad y funciones previstas."),
}


def digest(path):
    h = hashlib.sha256()
    with path.open("rb") as f:
        for block in iter(lambda: f.read(1024 * 1024), b""):
            h.update(block)
    return {"bytes": path.stat().st_size, "sha256": h.hexdigest()}


def render_doc(key, body):
    title = GUIDES[key][0] if key in GUIDES else "Documentación"
    label = "CENTRO DE AYUDA" if key == "INDEX" else f"GUÍA / {key}"
    back = "" if key == "INDEX" else '<p><a href="./">← Volver a todas las guías</a></p>'
    navigation = "".join(
        f'<a class="{"current" if item == key else ""}" href="{item}.html"'
        f'{" aria-current=\"page\"" if item == key else ""}>{html.escape(info[0])}</a>'
        for item, info in GUIDES.items()
    )
    return f'''<!doctype html>
<html lang="es"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1"><meta name="theme-color" content="#0b111c"><title>{html.escape(title)} · Mortymel Matrix</title><link rel="stylesheet" href="../styles.css"></head>
<body><a class="skip" href="#contenido">Saltar al contenido</a><header class="topbar"><div class="shell topbar-inner"><a class="wordmark" href="../"><span class="mark" aria-hidden="true">▦</span> MORTYMEL <span>MATRIX</span></a><nav aria-label="Navegación principal"><a href="../">Instalador</a><a class="active" href="./">Documentación</a><a href="https://github.com/Mortymel/mortymel-matrix">GitHub ↗</a></nav></div></header>
<main class="shell doc-layout" id="contenido"><aside class="doc-nav" aria-label="Guías"><span>DOCUMENTACIÓN</span><a href="./">Todas las guías</a>{navigation}</aside><article class="doc-content"><p class="eyebrow">{html.escape(label)}</p>{body}{back}</article></main><footer><div class="shell footer-inner"><span>MORTYMEL MATRIX <span class="foot-dim">/ Código MIT</span></span><a href="../">Instalar ↗</a></div></footer></body></html>'''


def main():
    framework = Path.home() / ".platformio/packages/framework-arduinoespressif32"
    firmware = SITE / "firmware"
    firmware.mkdir(parents=True, exist_ok=True)
    artifacts = {}
    for capacity, (environment, stem) in PROFILES.items():
        build = ROOT / ".pio/build" / environment
        inputs = [build / "bootloader.bin", build / "partitions.bin",
                  framework / "tools/partitions/boot_app0.bin", build / "firmware.bin"]
        missing = [str(path) for path in inputs if not path.is_file()]
        if missing:
            raise SystemExit("Archivos de compilación ausentes: " + ", ".join(missing))
        if any(path.read_bytes()[:1] != b"\xe9" for path in (inputs[0], inputs[3])):
            raise SystemExit(f"Bootloader o aplicación inválidos: {environment}")
        if inputs[3].stat().st_size > APP_SLOTS[capacity]:
            raise SystemExit(f"La aplicación supera la partición OTA de {capacity} MB")
        usb = firmware / f"{stem}.bin"
        ota = firmware / f"{stem}-ota.bin"
        subprocess.run([
            sys.executable, "-m", "esptool", "--chip", "esp32s3", "merge_bin",
            "-o", str(usb), "--flash_mode", "dio", "--flash_freq", "40m",
            "--flash_size", f"{capacity}MB", "0x0", str(inputs[0]),
            "0x8000", str(inputs[1]), "0xe000", str(inputs[2]),
            "0x10000", str(inputs[3]),
        ], check=True)
        if usb.stat().st_size > int(capacity) * 1024 * 1024 or usb.read_bytes()[0] != 0xe9:
            raise SystemExit(f"La imagen USB no cabe en {capacity} MB o no tiene cabecera")
        shutil.copyfile(inputs[3], ota)
        artifacts[capacity] = {"usb": {"path": f"firmware/{usb.name}", **digest(usb)},
                               "ota": {"path": f"firmware/{ota.name}", **digest(ota)}}
    for name in ("index.html", "styles.css", "site.js"):
        shutil.copyfile(ROOT / "flasher" / name, SITE / name)
    (SITE / "docs").mkdir(exist_ok=True)
    cards = "".join(
        f'<a href="{key}.html"><strong>{html.escape(title)} →</strong><span>{html.escape(summary)}</span></a>'
        for key, (title, summary) in GUIDES.items()
    )
    (SITE / "docs/index.html").write_text(render_doc("INDEX", f'<h1>Documentación</h1><p>Guías para instalar, conectar y ampliar Mortymel Matrix.</p><div class="doc-grid">{cards}</div>'), encoding="utf-8")
    for key in GUIDES:
        source = (ROOT / "docs" / f"{key}.md").read_text(encoding="utf-8")
        body = markdown.markdown(source, extensions=["extra", "toc"])
        body = re.sub(r'href="([A-Z]+)\.md"', r'href="\1.html"', body)
        (SITE / "docs" / f"{key}.html").write_text(render_doc(key, body), encoding="utf-8")

    for capacity in PROFILES:
        manifest = {
            "name": f"Mortymel Matrix · ESP32-S3 QD {capacity} MB", "version": VERSION,
            "new_install_prompt_erase": True, "new_install_improv_wait_time": 0,
            "builds": [{"chipFamily": "ESP32-S3", "improv": False,
                        "parts": [{"path": artifacts[capacity]["usb"]["path"], "offset": 0}]}],
        }
        output = json.dumps(manifest, ensure_ascii=False, indent=2) + "\n"
        (SITE / f"manifest-{capacity}mb.json").write_text(output, encoding="utf-8")
        if capacity == "8":
            (SITE / "manifest.json").write_text(output, encoding="utf-8")  # old links
    info = {"version": VERSION, "commit": os.environ.get("GITHUB_SHA", "local")[:12],
            "profiles": artifacts}
    (SITE / "build.json").write_text(json.dumps(info, indent=2) + "\n", encoding="utf-8")
    print(f"Sitio listo: {SITE} · {VERSION} · perfiles QD: {', '.join(PROFILES)} MB")


if __name__ == "__main__":
    main()
