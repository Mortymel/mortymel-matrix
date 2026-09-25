"""Assemble the USB image and ESP Web Tools site for the supported S3 profile."""
import json
import shutil
import subprocess
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[1]
build = root / ".pio/build/esp32-s3"
site = root / "dist"
target = site / "firmware/mortymel-matrix-s3-n8.bin"
framework = Path.home() / ".platformio/packages/framework-arduinoespressif32"
inputs = [build / "bootloader.bin", build / "partitions.bin", framework / "tools/partitions/boot_app0.bin", build / "firmware.bin"]
missing = [str(path) for path in inputs if not path.is_file()]
if missing:
    raise SystemExit("Archivos de compilación ausentes: " + ", ".join(missing))
target.parent.mkdir(parents=True, exist_ok=True)
subprocess.run([
    sys.executable, "-m", "esptool", "--chip", "esp32s3", "merge_bin",
    "-o", str(target), "--flash_mode", "dio", "--flash_freq", "40m", "--flash_size", "8MB",
    "0x0", str(inputs[0]), "0x8000", str(inputs[1]),
    "0xe000", str(inputs[2]), "0x10000", str(inputs[3]),
], check=True)
shutil.copyfile(root / "flasher/index.html", site / "index.html")
manifest = {
    "name": "Mortymel Matrix · ESP32-S3 8 MB",
    "version": "0.2.0",
    "new_install_prompt_erase": True,
    "new_install_improv_wait_time": 0,
    "builds": [{"chipFamily": "ESP32-S3", "improv": False,
                "parts": [{"path": "firmware/mortymel-matrix-s3-n8.bin", "offset": 0}]}],
}
(site / "manifest.json").write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n")
print(f"Sitio listo: {site} ({target.stat().st_size} bytes de firmware)")
