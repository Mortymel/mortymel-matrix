#!/usr/bin/env python3
"""Build manifests and a manual board catalog for ESP32, S2 and S3."""
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
PROFILES = {
    "wroom-clockwise-4": ("wroom-clockwise", "mortys-wroom-clockwise-4mb", "ESP32", 4, "esp32", 0x1000),
    "s3-4": ("s3-4mb", "mortymel-matrix-s3-4mb", "ESP32-S3", 4, "esp32s3", 0),
    "s3-8": ("esp32-s3", "mortymel-matrix-s3-n8", "ESP32-S3", 8, "esp32s3", 0),
    "s3-16": ("s3-16mb", "mortymel-matrix-s3-16mb", "ESP32-S3", 16, "esp32s3", 0),
    "esp32-4": ("esp32-classic", "mortymel-matrix-esp32-4mb", "ESP32", 4, "esp32", 0x1000),
    "s2-4": ("esp32-s2", "mortymel-matrix-s2-4mb", "ESP32-S2", 4, "esp32s2", 0x1000),
    "esp32-8": ("esp32-8mb", "mortymel-matrix-esp32-8mb", "ESP32", 8, "esp32", 0x1000),
    "esp32-16": ("esp32-16mb", "mortymel-matrix-esp32-16mb", "ESP32", 16, "esp32", 0x1000),
    "s2-8": ("s2-8mb", "mortymel-matrix-s2-8mb", "ESP32-S2", 8, "esp32s2", 0x1000),
    "s2-16": ("s2-16mb", "mortymel-matrix-s2-16mb", "ESP32-S2", 16, "esp32s2", 0x1000),
}
SITE = ROOT / "dist"
VERSION = (ROOT / "VERSION").read_text().strip()
GUIDES = {
    "IMAGENES": ("Fotos y GIF", "Preparar imágenes, animaciones y archivos compatibles con el panel."),
    "INSTALACION": ("Instalación y recuperación", "Primer arranque, Wi-Fi, panel y actualizaciones."),
    "COMPATIBILIDAD": ("Placas y perfiles", "Módulos, memorias y límites del firmware HUB75."),
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


def asset_name(name):
    source = ROOT / "flasher" / name
    return f"{source.stem}.{digest(source)['sha256'][:12]}{source.suffix}"


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
<html lang="es"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1"><meta name="theme-color" content="#f4f3ec"><title>{html.escape(title)} · Morty´s</title><script src="../{asset_name("theme.js")}"></script><link rel="stylesheet" href="../{asset_name("styles.css")}"></head>
<body><a class="skip" href="#contenido">Saltar al contenido</a><header class="topbar"><div class="shell topbar-inner"><a class="wordmark" href="../"><span class="mark" aria-hidden="true">m<span></span></span>Morty´s<span class="wordmark-tag">PIXEL LAB</span></a><nav aria-label="Navegación principal"><a href="../">Instalador</a><a class="active" href="./">Documentación</a><a href="https://github.com/Mortymel/mortymel-matrix">GitHub ↗</a><button class="theme-toggle" id="theme-toggle" type="button" aria-label="Activar tema oscuro" title="Activar tema oscuro"><span data-theme-icon aria-hidden="true">◐</span><span data-theme-label>Oscuro</span></button></nav></div></header>
<main class="shell doc-layout" id="contenido"><aside class="doc-nav" aria-label="Guías"><span>DOCUMENTACIÓN</span><a href="./">Todas las guías</a>{navigation}</aside><article class="doc-content"><p class="eyebrow">{html.escape(label)}</p>{body}{back}</article></main><footer><div class="shell footer-inner"><span>MORTY´S <span class="foot-dim">/ Código MIT</span></span><a href="../">Instalar ↗</a></div></footer></body></html>'''


def render_console_preview():
    page = (ROOT / "web/index.html").read_text(encoding="utf-8")
    page = page.replace("<body>", '<body><div style="text-align:center;padding:8px;background:var(--soft);color:var(--muted);font:11px Arial">Demostración · no controla un ESP32</div>', 1)
    mock = r"""
const demoState = {wifi:true,matrix:true,mode:'clock',message:'Hola',gif:'',brightness:96,ssid:'',timezone:'AST4',mqtt:false,mqtt_host:'',mqtt_port:1883,mqtt_user:'',device:'matrix_DEMO',chip_family:'ESP32',flash_bytes:4194304,display_profile:'WROOM · cableado Clockwise',clock_phase:false,time_synced:true,local_time:'17:45',brightness_schedule:{enabled:false,start:'22:00',end:'07:00',brightness:16,active:false},pins:{r1:25,g1:26,b1:27,r2:14,g2:12,b2:13,a:23,b:19,c:5,d:17,e:18,lat:4,oe:15,clk:16}};
window.fetch = async (url, options={}) => {
  let data={},status=200;
  if (url==='/api/status') data=demoState;
  else if (url==='/api/media' && !options.method) data={files:[]};
  else if (url==='/api/brightness-schedule') {demoState.brightness_schedule={...JSON.parse(options.body),active:false};data={ok:true};}
  else if (url==='/api/display') {Object.assign(demoState,JSON.parse(options.body));data={ok:true};}
  else {status=400;data={error:'Esta opción requiere conectarse a tu ESP32.'};}
  return new Response(JSON.stringify(data),{status,headers:{'Content-Type':'application/json'}});
};
"""
    before, script = page.rsplit("<script>", 1)
    return before + "<script>" + mock + script


def main():
    framework = Path.home() / ".platformio/packages/framework-arduinoespressif32"
    firmware = SITE / "firmware"
    firmware.mkdir(parents=True, exist_ok=True)
    artifacts = {}
    for key, (environment, stem, family, capacity, esptool_chip, boot_offset) in PROFILES.items():
        build = ROOT / ".pio/build" / environment
        inputs = [build / "bootloader.bin", build / "partitions.bin",
                  framework / "tools/partitions/boot_app0.bin", build / "firmware.bin"]
        missing = [str(path) for path in inputs if not path.is_file()]
        if missing:
            raise SystemExit("Archivos de compilación ausentes: " + ", ".join(missing))
        if any(path.read_bytes()[:1] != b"\xe9" for path in (inputs[0], inputs[3])):
            raise SystemExit(f"Bootloader o aplicación inválidos: {environment}")
        if inputs[3].stat().st_size > APP_SLOTS[str(capacity)]:
            raise SystemExit(f"La aplicación supera la partición OTA de {capacity} MB")
        usb = firmware / f"{stem}.bin"
        ota = firmware / f"{stem}-ota.bin"
        subprocess.run([
            sys.executable, "-m", "esptool", "--chip", esptool_chip, "merge_bin",
            "-o", str(usb), "--flash_mode", "dio", "--flash_freq", "40m",
            "--flash_size", f"{capacity}MB", hex(boot_offset), str(inputs[0]),
            "0x8000", str(inputs[1]), "0xe000", str(inputs[2]),
            "0x10000", str(inputs[3]),
        ], check=True)
        if usb.stat().st_size > capacity * 1024 * 1024 or usb.read_bytes()[boot_offset] != 0xe9:
            raise SystemExit(f"La imagen USB no cabe en {capacity} MB o no tiene cabecera")
        shutil.copyfile(inputs[3], ota)
        artifacts[key] = {"chipFamily": family, "flashMB": capacity, "bootOffset": boot_offset, "usb": {"path": f"firmware/{usb.name}", **digest(usb)},
                               "ota": {"path": f"firmware/{ota.name}", **digest(ota)}}
    artifacts["wroom-clockwise-4"]["label"] = "WROOM · cableado Clockwise · perfil 4 MB"
    for name in ("index.html", "styles.css", "site.js", "profile.mjs", "theme.js"):
        shutil.copyfile(ROOT / "flasher" / name, SITE / name)
    # Content-addressed theme assets prevent a new button using cached old CSS.
    for name in ("styles.css", "theme.js"):
        shutil.copyfile(ROOT / "flasher" / name, SITE / asset_name(name))
    page = (SITE / "index.html").read_text(encoding="utf-8")
    for name in ("styles.css", "theme.js"):
        page = page.replace(f'"./{name}"', f'"./{asset_name(name)}"')
    (SITE / "index.html").write_text(page, encoding="utf-8")
    (SITE / "console-preview.html").write_text(render_console_preview(), encoding="utf-8")
    (SITE / "docs").mkdir(exist_ok=True)
    cards = "".join(
        f'<a href="{key}.html"><strong>{html.escape(title)} →</strong><span>{html.escape(summary)}</span></a>'
        for key, (title, summary) in GUIDES.items()
    )
    (SITE / "docs/index.html").write_text(render_doc("INDEX", f'<h1>Documentación</h1><p>Guías para instalar, conectar y ampliar Morty´s.</p><div class="doc-grid">{cards}</div>'), encoding="utf-8")
    for key in GUIDES:
        source = (ROOT / "docs" / f"{key}.md").read_text(encoding="utf-8")
        body = markdown.markdown(source, extensions=["extra", "toc"])
        body = re.sub(r'href="([A-Z]+)\.md"', r'href="\1.html"', body)
        (SITE / "docs" / f"{key}.html").write_text(render_doc(key, body), encoding="utf-8")

    for key, (_, _, family, capacity, _, _) in PROFILES.items():
        manifest = {
            "name": artifacts[key].get("label", f"Morty´s · {family} {capacity} MB"), "version": VERSION,
            "new_install_prompt_erase": True, "new_install_improv_wait_time": 0,
            "builds": [{"chipFamily": family, "improv": False,
                        "parts": [{"path": artifacts[key]["usb"]["path"], "offset": 0}]}],
        }
        output = json.dumps(manifest, ensure_ascii=False, indent=2) + "\n"
        (SITE / f"manifest-{key}.json").write_text(output, encoding="utf-8")
        if family == "ESP32-S3":
            (SITE / f"manifest-{capacity}mb.json").write_text(output, encoding="utf-8")
        if key == "s3-8":
            (SITE / "manifest.json").write_text(output, encoding="utf-8")  # old links
    info = {"version": VERSION, "commit": os.environ.get("GITHUB_SHA", "local")[:12],
            "profiles": artifacts, "boards": json.loads((ROOT / "flasher/boards.json").read_text(encoding="utf-8"))}
    (SITE / "build.json").write_text(json.dumps(info, indent=2) + "\n", encoding="utf-8")
    print(f"Sitio listo: {SITE} · {VERSION} · perfiles: {', '.join(PROFILES)}")


if __name__ == "__main__":
    main()
