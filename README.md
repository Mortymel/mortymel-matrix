# Mortymel Matrix

Firmware abierto para una matriz HUB75 de 64 × 64 controlada por ESP32-S3. Incluye una consola web local para reloj digital, texto y GIF, y control opcional desde Home Assistant mediante MQTT. Su estética recuerda a los relojes de píxeles, pero el código y la interfaz son originales; no incorpora esferas ni recursos de Clockwise.

**[Instalar desde el navegador](https://mortymel.github.io/mortymel-matrix/) · [Leer documentación](https://mortymel.github.io/mortymel-matrix/docs/)**

## Estado y compatibilidad

| Componente | Estado |
| --- | --- |
| Firmware e instalador USB | Compilan y se publican automáticamente. |
| Placa precompilada | ESP32-S3 con flash **QD de 4, 8 o 16 MB**; selecciona la capacidad real de tu módulo. |
| Panel | Una HUB75 64 × 64 de barrido 1/32 compatible, tras verificar alimentación y mapa de GPIO. |
| Primera configuración | AP temporal, credenciales aleatorias visibles a 115200 por USB nativo o USB-UART, web local con autenticación. |
| Contenido | Reloj digital, mensaje y GIF de hasta 64 × 64 y 400 KB cada uno. |
| Home Assistant | MQTT Discovery para escena, mensaje, brillo y estado. Requiere un broker. |
| Validación física | Pendiente con la placa y el panel concretos. |

Los perfiles publicados son para **S3 con flash QD**, no para cualquier ESP32 ni variantes OPI. El firmware arranca con la matriz desactivada. Las 14 señales se configuran en la web local y se deben cotejar con el montaje real. Consulta la [guía de conexión física](docs/CABLEADO.md) con la tabla completa de señales, 5 V, tierra y traductor lógico. El instalador no detecta si el panel conectado tiene el barrido ni el controlador esperados.

## Primeros pasos

1. Abre el [instalador HTTPS](https://mortymel.github.io/mortymel-matrix/) en Chrome o Edge de escritorio, conecta la placa por un cable USB de datos e instala el firmware.
2. Abre **Logs & Console** a 115200 y pulsa RESET para leer el nombre y contraseña del AP, y la contraseña web. Según la placa puede haber USB nativo, USB-UART o ambos.
3. Conéctate al AP `Mortymel-XXXXXX` y entra en `http://192.168.4.1/` con usuario `admin`. Configura Wi-Fi y cambia la contraseña de administración.
4. Comprueba los 14 GPIO, la alimentación y el tipo de panel con la [guía de cableado](docs/CABLEADO.md); configura el mapa y después conecta y asigna E en **Sistema**. Sube GIF desde **Biblioteca GIF**; opcionalmente configura MQTT en **Red y MQTT**.

El AP se cierra al conectar a Wi-Fi y vuelve a abrirse si se pierde la conexión. La web del dispositivo usa HTTP y está pensada para una red local confiable; no la publiques en Internet. Consulta la [guía de instalación y recuperación](docs/INSTALACION.md) si algún paso falla.

## Descargas USB y OTA

Para cada capacidad seleccionada, la página publica **dos archivos distintos**: la imagen fusionada para instalar por USB desde offset 0 y el `.bin` de la aplicación para actualizar en la consola local, sección **Sistema**. No subas la imagen USB por OTA. La página muestra versión, tamaños y SHA-256 de ambos archivos.

## Documentación

| Guía | Tema |
| --- | --- |
| [Instalación](docs/INSTALACION.md) | Hardware, primer arranque y recuperación. |
| [Cableado HUB75](docs/CABLEADO.md) | 14 señales, fuente, GND y secuencia de conexión. |
| [MQTT](docs/MQTT.md) | Entidades y automatización en Home Assistant. |
| [API web](docs/API.md) | Endpoints y límites. |
| [Arquitectura](docs/ARQUITECTURA.md) | Componentes y comportamiento. |
| [Desarrollo](docs/DESARROLLO.md) | Compilar, empaquetar y verificar. |
| [Hoja de ruta](docs/ROADMAP.md) | Trabajo pendiente. |

## Límites

Esta versión aún no ofrece soporte para todo ESP32 o todos los controladores HUB75, sensores, esferas de Clockwise ni subida de GIF dentro de la interfaz de Home Assistant. La vista previa web es aproximada y la reproducción física está pendiente de pruebas con hardware. Los GIF se cargan en la web local sin reflashear.

Código MIT de Mortymel. Bibliotecas de terceros: [HUB75 DMA](https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA), [AnimatedGIF](https://github.com/bitbank2/AnimatedGIF), Adafruit GFX, PubSubClient y ArduinoJson. Clockwise es de [jnthas](https://github.com/jnthas/clockwise); no se reutilizan sus archivos en este repositorio.
