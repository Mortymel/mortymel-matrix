# Instalación y recuperación

## Antes de empezar

El instalador publica perfiles para **ESP32-S3 con flash QD de 4, 8 o 16 MB**. Elige la capacidad real de tu módulo; no emplees estos binarios en placas OPI u otra familia de ESP32. Los GPIO expuestos dependen de la placa, no del tamaño de flash. Se necesita un cable USB de datos, Chrome o Edge de escritorio y una conexión a la página por HTTPS. El ESP32 usa Wi-Fi de 2,4 GHz. Para Home Assistant hace falta un broker MQTT accesible desde esa red.

Una matriz HUB75 64 × 64 necesita fuente externa de 5 V adecuada y tierra común con el ESP32. Consulta la [guía física HUB75](CABLEADO.md) para conectar las 14 señales, identificar IN, elegir GPIO y preparar la alimentación. Este proyecto inicia con la salida de matriz desactivada (`E = -1`). Puedes cambiar **todo el mapa** desde la web antes de cablear el panel.

## Primera instalación USB

1. Entra al [instalador Mortymel Matrix](https://mortymel.github.io/mortymel-matrix/) desde Chrome o Edge de escritorio.
2. Conecta el puerto USB serie disponible en tu placa S3, pulsa **Conectar e instalar** y elige el puerto que corresponda. Si existe otro programa que usa ese puerto, ciérralo.
3. El instalador puede ofrecer borrar la memoria: esto elimina ajustes, contraseñas y GIF anteriores. Continúa solo si quieres una instalación nueva. Espera a que termine el flasheo.
4. Abre **Logs & Console** en el instalador a 115200 baudios y pulsa RESET. Se imprimen el AP y su clave, además del usuario `admin` y su contraseña web inicial. Si no aparecen, prueba el otro puerto USB o abre el monitor de PlatformIO. Las claves se vuelven a imprimir al reiniciar.
5. Conéctate al AP `Mortymel-XXXXXX` y visita `http://192.168.4.1/`. Inicia sesión, configura Wi-Fi en **Red y MQTT** y cambia la contraseña web en **Sistema**.
6. Cuando el ESP32 se conecta a Wi-Fi, cierra el AP. La dirección IP local se imprime por serie. Si luego falla la red, el AP se reactiva para recuperación.

Se generan contraseñas aleatorias distintas para AP y web. Se guardan en NVS; reiniciar no las cambia. No publiques la interfaz HTTP en Internet.

## Activar la matriz

Configura y comprueba las 14 señales en **Sistema** siguiendo [Conexión física HUB75](CABLEADO.md). Con todo apagado, conecta el panel y su fuente; después define el GPIO real de E y guarda. El dispositivo se reinicia. Si falla, vuelve a E `-1` y revisa el cableado, la alimentación, el barrido y el controlador. La web informa si el controlador de pantalla pudo iniciarse, pero ese estado **no confirma** que la imagen se vea correctamente.

## Integrar Home Assistant

1. Configura un broker MQTT y la [integración MQTT de Home Assistant](https://www.home-assistant.io/integrations/mqtt/) para ese broker.
2. En la consola del ESP32 abre **Red y MQTT** y guarda servidor, puerto y credenciales. La pantalla de inicio indica si MQTT está conectado.
3. La integración recibirá por Discovery las entidades `Escena`, `Mensaje`, `Brillo` y `Estado`. Sube los GIF desde la web local del ESP32 y elige allí el archivo. [Detalles de temas y ejemplo](MQTT.md).

## Actualizar sin borrar configuración

En la [página del instalador](https://mortymel.github.io/mortymel-matrix/#descargas) descarga **firmware OTA (aplicación solamente)**. En la web local del ESP32 abre **Sistema → Actualización por Wi-Fi**, selecciona ese `.bin` y espera al reinicio. La imagen **USB completa** se escribe desde offset 0 y **no sirve para OTA**. Usa siempre el archivo OTA **de la misma capacidad y distribución de particiones** con la que instalaste el firmware; al cambiar de perfil reinstala por USB y respalda previamente ajustes y GIF. Las preferencias y los GIF se conservan en una actualización OTA normal.

## Compilar manualmente

Abre el proyecto con PlatformIO y revisa `platformio.ini`. Para las capacidades QD disponibles ejecuta `python scripts/embed_web.py` y `pio run -e s3-4mb`, `pio run -e esp32-s3` o `pio run -e s3-16mb`. Para otra memoria o familia modifica y verifica placa, flash y particiones antes de compilar. Los GPIO se ajustan desde la consola local. Para subir manualmente un perfil, con la placa conectada usa `pio run -e NOMBRE_DEL_PERFIL -t upload` (por ejemplo `esp32-s3` para 8 MB). Abre `pio device monitor -b 115200` en el puerto correcto. El archivo de aplicación queda en `.pio/build/esp32-s3/firmware.bin`.

## Recuperación

- **El puerto no aparece:** prueba un cable de datos, cambia de conector, libera el puerto en otras aplicaciones. Para entrar en modo descarga, mantén BOOT, pulsa RESET y suelta BOOT.
- **No hay registro de arranque:** abre el monitor a 115200 antes de pulsar RESET; la placa puede presentar dos puertos serie distintos.
- **Wi-Fi incorrecto:** espera a que vuelva a aparecer el AP temporal, conéctate y corrige la red desde la web local.
- **Panel oscuro:** deja E en `-1` hasta verificar pines, barrido, controlador y fuente. No se ha probado el panel concreto del usuario.
- **Firmware no inicia:** reinstala por USB. El borrado completo restaura el estado inicial, pero elimina NVS y GIF; no existe restauración automática de esos datos.
