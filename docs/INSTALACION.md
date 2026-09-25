# Instalación y recuperación

## Antes de empezar

El instalador precompilado corresponde a **ESP32-S3 DevKitC-1 N8**: flash QD de 8 MB, sin PSRAM. Se necesita un cable USB de datos, Chrome o Edge de escritorio y una conexión a la página por HTTPS. El ESP32 usa Wi-Fi de 2,4 GHz. Para Home Assistant hace falta un broker MQTT accesible desde esa red.

Una matriz HUB75 64 × 64 necesita alimentación externa adecuada y tierra común con el ESP32. Comprueba su controlador, barrido y todos los GPIO antes de conectarla. Este proyecto inicia con la salida de matriz desactivada (`GPIO E = -1`); solo E es configurable desde la web, y las demás señales usan el mapa predeterminado de la biblioteca HUB75 para S3. Ese mapa **no es universal**. No alimentes la matriz desde el pin 5 V del ESP32.

## Primera instalación USB

1. Entra al [instalador Mortymel Matrix](https://mortymel.github.io/mortymel-matrix/) desde Chrome o Edge de escritorio.
2. Conecta el puerto USB nativo **USB** o el conector **USB-UART** de la DevKitC-1, pulsa **Conectar e instalar** y elige el puerto que corresponda. Si existe otro programa que usa ese puerto, ciérralo.
3. El instalador puede ofrecer borrar la memoria: esto elimina ajustes, contraseñas y GIF anteriores. Continúa solo si quieres una instalación nueva. Espera a que termine el flasheo.
4. Abre **Logs & Console** en el instalador a 115200 baudios y pulsa RESET. Se imprimen el AP y su clave, además del usuario `admin` y su contraseña web inicial. Si no aparecen, prueba el otro puerto USB o abre el monitor de PlatformIO. Las claves se vuelven a imprimir al reiniciar.
5. Conéctate al AP `Mortymel-XXXXXX` y visita `http://192.168.4.1/`. Inicia sesión, configura Wi-Fi en **Red y MQTT** y cambia la contraseña web en **Sistema**.
6. Cuando el ESP32 se conecta a Wi-Fi, cierra el AP. La dirección IP local se imprime por serie. Si luego falla la red, el AP se reactiva para recuperación.

Se generan contraseñas aleatorias distintas para AP y web. Se guardan en NVS; reiniciar no las cambia. No publiques la interfaz HTTP en Internet.

## Activar la matriz

Solo después de comprobar el cableado completo y la compatibilidad del barrido, abre **Sistema**, introduce el GPIO conectado a E y guarda. El dispositivo se reinicia. Si la matriz no funciona, vuelve a `-1` y revisa todos los pines predeterminados, la alimentación y el controlador; para cambiar otras señales hay que crear un perfil y compilar. La web informa si el controlador de pantalla pudo iniciarse, pero ese estado **no confirma** que la imagen se vea correctamente.

## Integrar Home Assistant

1. Configura un broker MQTT y la [integración MQTT de Home Assistant](https://www.home-assistant.io/integrations/mqtt/) para ese broker.
2. En la consola del ESP32 abre **Red y MQTT** y guarda servidor, puerto y credenciales. La pantalla de inicio indica si MQTT está conectado.
3. La integración recibirá por Discovery las entidades `Escena`, `Mensaje`, `Brillo` y `Estado`. Sube los GIF desde la web local del ESP32 y elige allí el archivo. [Detalles de temas y ejemplo](MQTT.md).

## Actualizar sin borrar configuración

En la [página del instalador](https://mortymel.github.io/mortymel-matrix/#descargas) descarga **firmware OTA (aplicación solamente)**. En la web local del ESP32 abre **Sistema → Actualización por Wi-Fi**, selecciona ese `.bin` y espera al reinicio. La imagen **USB completa** se escribe desde offset 0 y **no sirve para OTA**. Usa siempre un firmware correspondiente al perfil de placa instalado. Las preferencias y los GIF se conservan en una actualización OTA normal.

## Compilar manualmente

Abre el proyecto con PlatformIO y revisa `platformio.ini`. Para un hardware diferente, modifica primero el perfil de placa, tamaño y modo de flash, memoria y mapa de GPIO. Ejecuta `python scripts/embed_web.py`, `pio run -e esp32-s3` y, con la DevKitC-1 conectada, `pio run -e esp32-s3 -t upload`. Abre `pio device monitor -b 115200` en el puerto correcto. El archivo de aplicación queda en `.pio/build/esp32-s3/firmware.bin`.

## Recuperación

- **El puerto no aparece:** prueba un cable de datos, cambia de conector, libera el puerto en otras aplicaciones. Para entrar en modo descarga, mantén BOOT, pulsa RESET y suelta BOOT.
- **No hay registro de arranque:** abre el monitor a 115200 antes de pulsar RESET; la placa puede presentar dos puertos serie distintos.
- **Wi-Fi incorrecto:** espera a que vuelva a aparecer el AP temporal, conéctate y corrige la red desde la web local.
- **Panel oscuro:** deja E en `-1` hasta verificar pines, barrido, controlador y fuente. No se ha probado el panel concreto del usuario.
- **Firmware no inicia:** reinstala por USB. El borrado completo restaura el estado inicial, pero elimina NVS y GIF; no existe restauración automática de esos datos.
