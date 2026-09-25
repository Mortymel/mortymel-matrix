# Mortymel Matrix

Firmware original para ESP32-S3 y matrices HUB75 64×64. Su consola oscura se
inspira en el estilo de los relojes de píxeles de Clockwise. Este repositorio
**no incorpora código, imágenes ni esferas del proyecto Clockwise**.

## Documentación

| Guía | Contenido |
| --- | --- |
| [Instalación](docs/INSTALACION.md) | Primera carga, red, Home Assistant y recuperación. |
| [Arquitectura](docs/ARQUITECTURA.md) | Componentes, almacenamiento y límites del prototipo. |
| [API web](docs/API.md) | Rutas, formatos y validaciones. |
| [MQTT](docs/MQTT.md) | Discovery, temas y automatización de ejemplo. |
| [Desarrollo](docs/DESARROLLO.md) | Archivos, cambios y verificación. |
| [Hoja de ruta](docs/ROADMAP.md) | Funciones y compatibilidad pendientes. |

## Primera versión

- Compilación automática de firmware en GitHub Actions para cambios nuevos.
- Un único firmware inicial, servidor web local en el ESP32 y asistente de Wi-Fi.
- Reloj digital, texto editable, reproducción de GIF 64×64 desde LittleFS.
- Subida, listado y borrado de GIF por HTTP; ajustes persistentes en Preferences.
- MQTT opcional con Discovery de Home Assistant: texto, escena, brillo, y estado.
- Actualización de firmware `.bin` mediante la página web (OTA).
- Zona horaria POSIX configurable desde la web (por ejemplo, `EST5`).
- Perfil de pantalla separable del resto: sin `MATRIX_E_PIN` válido arranca la
  consola web, pero deja la matriz desactivada. Esto evita prometer un cableado
  universal inexistente para las placas S3.

## Instalación

1. Instala [PlatformIO](https://platformio.org/install) y abre esta carpeta.
2. En `platformio.ini`, selecciona la variante y memoria de tu placa; asigna
   `-DMATRIX_E_PIN=GPIO` al GPIO que conecta la línea E de tu panel 1/32 scan.
   Los demás pines usan los predeterminados de la biblioteca HUB75 para S3;
   el perfil real debe verificarse antes de conectar el panel.
3. Compila y carga una vez con `pio run -t upload`.
4. Abre el monitor serie a 115200: imprime el nombre y contraseña aleatoria
   del punto de acceso inicial y la contraseña inicial de administración.
5. Conéctate al punto de acceso, visita `http://192.168.4.1/`, inicia sesión
   y configura Wi-Fi. Después visita la IP que muestra el monitor serie.
6. Configura el broker MQTT en la web y activa la integración MQTT de Home
   Assistant. Las entidades se publican con MQTT Discovery.

La contraseña inicial de administración se imprime por serie al arrancar hasta
que se cambia desde la web. Cambia esa contraseña al configurar el dispositivo.
La interfaz HTTP está diseñada para una red local confiable: no abras el puerto
80 del ESP32 a Internet. El AP de recuperación se activa cuando falta Wi-Fi.

## Arquitectura

`src/main.cpp`: pantalla, GIF, web, MQTT, almacenamiento, OTA.

`include/web_ui.h`: consola web autocontenida, incluida en el firmware.

`web/`: fuente legible del frontend; `scripts/embed_web.py` crea `include/web_ui.h`.

`docs/ROADMAP.md`: compatibilidad de placas y funciones siguientes.

Los GIF se envían por HTTP y se guardan en LittleFS. MQTT transporta órdenes
cortas y estados. Los cambios de medios y ajustes no requieren recompilar;
añadir nuevas funciones sí requiere un firmware actualizado por OTA.

## Límites actuales

Primer prototipo para una sola matriz 64×64. El reloj digital y los GIF
requieren panel compatible y pines ajustados. No se han probado todavía en la
placa ni el panel concretos del usuario. No se implementan aún el motor de
esferas de Clockwise, sensores opcionales, listas de reproducción ni una app
que suba archivos directamente desde la interfaz de Home Assistant. La web
local del ESP32 ya permite subirlos sin reflashear.

Clockwise es de [jnthas](https://github.com/jnthas/clockwise), bajo MIT; sus
gráficos y personajes no se incluyen. Bibliotecas: [HUB75 DMA](https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA),
[AnimatedGIF](https://github.com/bitbank2/AnimatedGIF), PubSubClient y ArduinoJson.
