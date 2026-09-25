# Instalación inicial

## Requisitos

- Placa ESP32-S3. El perfil actual de PlatformIO es `esp32-s3-devkitc-1`.
- Matriz HUB75 de 64×64 compatible, normalmente 1/32 scan.
- Fuente de alimentación adecuada para la matriz y conexión de tierra común.
- Cable USB de datos y navegador Chrome o Edge de escritorio para la variante S3 DevKitC-1 de 8 MB; para otras variantes, Python y PlatformIO.
- Wi-Fi 2,4 GHz. Broker MQTT solo si se necesita Home Assistant.

## Identificar el perfil de pantalla

La biblioteca HUB75 usa un mapa predeterminado de GPIO para el chip S3, pero
esto **no equivale al pinout universal de cualquier placa S3**. En particular,
una matriz 64×64 1/32 scan necesita la señal E. En **Sistema**, definir GPIO E tras verificar el diseño de la placa. El firmware
de instalación inicia con E = -1 y no activa el panel. El resto de señales
usa los GPIO predeterminados de la biblioteca; para cambiarlos hay que compilar
un perfil específico. Guardar GPIO E reinicia el ESP32.

Antes de energizar el panel, consultar el fabricante sobre su tipo de barrido,
chip controlador y consumo. No usar el pin 5 V del ESP32 para alimentar la
matriz.

## Instalar desde el navegador

Para la ESP32-S3 DevKitC-1 de 8 MB, abrir
[Mortymel Matrix Installer](https://mortymel.github.io/mortymel-matrix/) con
Chrome o Edge de escritorio por HTTPS. Conectar por cable USB de datos, pulsar
**Conectar e instalar** y elegir el puerto. ESP Web Tools pide confirmación si
se van a borrar los datos. Abrir **Logs & Console** tras terminar y reiniciar
la placa para volver a ver las contraseñas iniciales. La página instala una
imagen fusionada para USB; no cargar esa imagen en OTA.

## Compilar y flashear manualmente

1. Abrir el directorio del proyecto en VS Code con PlatformIO.
2. Ajustar `board` a la variante real de S3 y su esquema de memoria.
3. Verificar los GPIO predeterminados de HUB75 y, si procede, ajustar `MATRIX_E_PIN` como valor inicial (también configurable en la web).
4. La página `include/web_ui.h` ya está incluida. Al editar
   `web/index.html`, ejecutar `python3 scripts/embed_web.py` antes de compilar.
5. Ejecutar `pio run -t upload` y abrir el monitor con `pio device monitor -b
   115200`. La primera instalación requiere USB.

## Entrar a la consola

Al primer arranque se generan contraseñas independientes para el punto de
acceso y para la web. El monitor serie imprime:

- `AP: Mortymel-XXXXXX`, contraseña y `http://192.168.4.1/`, cuando la placa
  todavía no tiene Wi-Fi funcional.
- `Usuario web: admin`, contraseña inicial o actual.

Conectarse al AP desde un teléfono o PC, abrir la URL, iniciar sesión y guardar
SSID y contraseña Wi-Fi en **Red y MQTT**. Si el ESP32 ya se conectó a la red,
el monitor mostrará su IP local. Cambiar la contraseña de administración en
**Sistema**. La contraseña del AP se conserva de forma persistente y se usa
para recuperar acceso cuando falla la conexión Wi-Fi.

## Home Assistant

1. Instalar o activar un broker MQTT accesible desde el ESP32.
2. Configurar la integración MQTT de Home Assistant con ese broker.
3. En **Red y MQTT**, introducir host, puerto, usuario y contraseña del broker.
4. Tras conectarse, el ESP32 publica descubrimiento MQTT para cuatro entidades
   del dispositivo: `Escena`, `Mensaje`, `Brillo`, `Estado`.
5. Usar las entidades en un panel o automatización. Los GIF se cargan en la
   web local del ESP32, en **Biblioteca GIF**.

## Actualizaciones siguientes

Desde **Sistema**, cargar solo el `.pio/build/esp32-s3/firmware.bin` de la compilación, nunca la imagen fusionada de instalación USB, compatible con exactamente la variante
instalada. La operación OTA necesita particiones de aplicación compatibles.
Verificar que el dispositivo inicia correctamente después. Los GIF, la escena,
el Wi-Fi y MQTT se conservan en el almacenamiento del equipo. No subir un
binario destinado a otra placa.

## Volver a un estado funcional

Si la red falla, consultar la consola serie y conectarse al AP de recuperación.
Si el firmware no inicia y OTA no funciona, el método de recuperación es
flashear de nuevo por USB. El prototipo aún no implementa una opción web de
restablecimiento ni prueba automatizada de rollback.
