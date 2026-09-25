# Guía de desarrollo

## Organización

```text
include/web_ui.h     Página compilada en el firmware
web/index.html       Fuente de la consola
src/main.cpp         Firmware y API
scripts/embed_web.py Genera include/web_ui.h
platformio.ini       Perfil inicial ESP32-S3
docs/                Instalación, API, MQTT, arquitectura y hoja de ruta
```

## Modificar interfaz

Editar `web/index.html`, ejecutar `python3 scripts/embed_web.py` y comprobar
la sintaxis del JavaScript. `include/web_ui.h` está versionado para permitir
compilar el proyecto sin una fase adicional de generación. La consola web
actual muestra una vista previa aproximada; no captura imágenes del panel.

## Modificar firmware

La entrada `esp32-s3` en `platformio.ini` es el perfil inicial. Antes de crear
un perfil nuevo, verificar barrido y chip de la matriz, GPIO disponibles,
memoria flash, tipo de PSRAM, Wi-Fi durante DMA y tabla de particiones OTA.
Mantener los nombres de temas MQTT y endpoints para preservar paneles de HO.

## Verificaciones mínimas

1. `python3 scripts/embed_web.py` genera un encabezado actual.
2. `pio run -e esp32-s3` compila sin panel configurado.
3. Con una placa conectada, `pio run -e esp32-s3 -t upload` permite acceder a
   la web, cambiar credenciales y configurar Wi-Fi.
4. Con un panel verificado, comprobar color, barrido, brillo, reloj y GIF.
5. Cargar un GIF de prueba y validar reinicio, borrado y selección.
6. Con broker local, comprobar MQTT Discovery, estados y comandos.
7. Probar OTA con particiones apropiadas; verificar arranque tras actualizar.

Se comprobó la generación de HTML y la sintaxis JS. GitHub Actions compila
`esp32-s3` con PlatformIO en cada envío; la primera compilación satisfactoria
se completó con el perfil de pantalla desactivada (`MATRIX_E_PIN=-1`). Las
pruebas en una placa y panel reales aún no se han ejecutado.

## Licencias

El código de este repositorio se distribuye bajo MIT como Mortymel Matrix.
Clockwise también es MIT, pero su código no forma parte de este prototipo.
Conservar los avisos de licencias de cada dependencia y revisar por separado
las licencias de fuentes, GIF, sprites y personajes que se distribuyan.
