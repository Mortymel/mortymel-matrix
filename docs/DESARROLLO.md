# Desarrollo y publicación

## Estructura

| Ruta | Propósito |
| --- | --- |
| `src/main.cpp` | Firmware, web local, GIF, MQTT, pantalla y OTA. |
| `web/index.html` | Fuente de la consola incluida en el firmware. |
| `include/web_ui.h` | Consola generada; se versiona para compilar sin fase adicional. |
| `flasher/` | Fuente HTML, CSS y JavaScript del instalador público. |
| `scripts/embed_web.py` | Genera `include/web_ui.h`. |
| `scripts/package_flasher.py` | Fusiona el firmware USB, copia OTA y genera guías y huellas. |
| `scripts/check_site.py` | Verifica imágenes, hashes, manifest y enlaces locales. |
| `docs/` | Guías Markdown fuente del sitio y del repositorio. |
| `VERSION` | Versión mostrada en el instalador y en su manifest. |

## Compilar y generar el sitio

Los perfiles `s3-4mb`, `esp32-s3` y `s3-16mb` cubren flash QD de 4/8/16 MB. Antes de dar soporte a otra variante, comprobar flash QD/OPI, PSRAM, particiones, GPIO, controlador y barrido. Consulta [Cableado](CABLEADO.md). Para cambiar la consola embebida, edita `web/index.html` y regenera el encabezado.

```bash
python -m pip install platformio esptool==4.8.1 Markdown==3.7
python scripts/embed_web.py
pio run -e esp32-s3
python scripts/package_flasher.py
python scripts/check_site.py
```

El resultado está en `dist/`: `index.html`, `docs/`, tres manifests por capacidad, `build.json`, imágenes fusionadas USB y archivos OTA por perfil. `manifest.json` sigue apuntando a 8 MB por compatibilidad. `dist/` es generado y no se versiona. La imagen S3 sitúa el bootloader en `0x0`, particiones en `0x8000`, selector OTA en `0xe000` y aplicación en `0x10000`; `check_site.py` comprueba la cabecera de la aplicación y SHA-256. La imagen USB no se usa para OTA. El perfil de 4 MB emplea dos particiones de aplicación de 0x1A0000 y 0xA0000 para LittleFS; el empaquetador rechaza un firmware que exceda cualquiera de las particiones OTA.

GitHub Actions ejecuta la misma compilación y verificación en `push` y `pull_request`. En `main` publica el sitio mediante GitHub Pages. El instalador usa ESP Web Tools, con el manifiesto y los binarios en el mismo origen HTTPS.

## Pruebas con hardware pendientes

1. Instalar los perfiles 4/8/16 MB en hardware S3 QD verificado; leer las claves a 115200 tras RESET.
2. Conectar AP, cambiar la clave web y pasar a Wi-Fi; verificar cierre y recuperación del AP.
3. Confirmar el mapa de pines y el barrido del panel antes de habilitar E. Revisar reloj, texto, color, brillo y GIF.
4. Comprobar límites y persistencia de GIF después de reiniciar; forzar un fallo de montaje sin perder datos.
5. Probar entidades MQTT y automatizaciones con un broker real de Home Assistant.
6. Actualizar con el `.bin` OTA publicado y confirmar reinicio y persistencia de preferencias y GIF.

Una compilación correcta y el despliegue público no sustituyen estas pruebas físicas.
