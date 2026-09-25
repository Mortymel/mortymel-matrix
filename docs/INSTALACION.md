# Instalación inicial

## Requisitos

- Placa ESP32-S3. El perfil actual de PlatformIO es `esp32-s3-devkitc-1`.
- Matriz HUB75 de 64×64 compatible, normalmente 1/32 scan.
- Fuente de alimentación adecuada para la matriz y conexión de tierra común.
- Python y PlatformIO para el primer flasheo; cable USB de datos.
- Wi-Fi 2,4 GHz. Broker MQTT solo si se necesita Home Assistant.

## Identificar el perfil de pantalla

La biblioteca HUB75 usa un mapa predeterminado de GPIO para el chip S3, pero
esto **no equivale al pinout universal de cualquier placa S3**. En particular,
una matriz 64×64 1/32 scan necesita la señal E. En `platformio.ini`, cambiar
`-DMATRIX_E_PIN=-1` por el GPIO conectado a E tras verificar el diseño de la
placa. La compilación sin ese cambio ofrece web y MQTT y reporta pantalla no
configurada. Otros GPIO solo se pueden cambiar tras definir un perfil de
placa específico: esta primera versión no incluye dicho selector web.

Antes de energizar el panel, consultar el fabricante sobre su tipo de barrido,
chip controlador y consumo. No usar el pin 5 V del ESP32 para alimentar la
matriz.

## Preparar y flashear

1. Abrir el directorio del proyecto en VS Code con PlatformIO.
2. Ajustar `board` a la variante real de S3 y su esquema de memoria.
3. Ajustar `MATRIX_E_PIN` si el perfil de GPIO restante es compatible.
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

Desde **Sistema**, cargar un `.bin` compatible con exactamente la variante
instalada. La operación OTA necesita particiones de aplicación compatibles.
Verificar que el dispositivo inicia correctamente después. Los GIF, la escena,
el Wi-Fi y MQTT se conservan en el almacenamiento del equipo. No subir un
binario destinado a otra placa.

## Volver a un estado funcional

Si la red falla, consultar la consola serie y conectarse al AP de recuperación.
Si el firmware no inicia y OTA no funciona, el método de recuperación es
flashear de nuevo por USB. El prototipo aún no implementa una opción web de
restablecimiento ni prueba automatizada de rollback.
