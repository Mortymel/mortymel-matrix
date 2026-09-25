# Arquitectura y límites

## Recorrido de los datos

| Origen | Destino | Uso |
| --- | --- | --- |
| Navegador local | ESP32 por HTTP | Configurar red, escenas y las 14 señales HUB75; subir y borrar GIF; actualizar aplicación. |
| Home Assistant | Broker MQTT | Enviar escena, mensaje y brillo; recibir estados publicados por el ESP32. |
| ESP32 | LittleFS | Leer y guardar GIF. |
| ESP32 | NVS / Preferences | Guardar red, contraseñas, MQTT, escena y mapa de GPIO HUB75. |
| ESP32 | HUB75 DMA | Dibujar reloj, texto o cuadros GIF en una matriz compatible. |

La página HTML, CSS y JavaScript de la consola se compila en `include/web_ui.h` y no requiere una carga separada del sistema de archivos. LittleFS se formatea solo en el primer arranque reconocido; un fallo posterior de montaje no borra automáticamente los GIF. El sitio del instalador en GitHub Pages es distinto de la consola local integrada en el firmware.

## Componentes

| Módulo | Función |
| --- | --- |
| WebServer | Interfaz HTTP y API con autenticación Basic. |
| WiFi / Preferences | Conexión Wi-Fi, AP de recuperación y ajustes persistentes. |
| PubSubClient | MQTT, comandos y descubrimiento para Home Assistant. |
| HUB75 MatrixPanel DMA | Salida de píxeles hacia una matriz 64 × 64 compatible. |
| AnimatedGIF / LittleFS | Decodificación y biblioteca local de GIF. |
| Update | Actualización de la partición de aplicación desde la consola local. |

El bucle principal atiende servidor HTTP, MQTT y pantalla cooperativamente. Un GIF complejo o una conexión lenta puede afectar la respuesta. El servidor local no ofrece HTTPS y debe usarse en una red de confianza.

## Compatibilidad

Los perfiles publicados cubren ESP32-S3 con flash QD de 4, 8 y 16 MB y ESP32 clásico/S2 con al menos 4 MB y una matriz 64 × 64 de barrido 1/32 compatible. Las 14 señales se pueden reubicar desde la web; para otros chips, flash OPI u otro barrido es necesario portar y compilar. El estado `matrix: true` significa que el controlador DMA inició; no garantiza que el panel, sus colores o su barrido sean correctos. El tamaño total de LittleFS depende de las particiones; 400 KB es un límite por archivo, no una garantía de capacidad libre.

Esta versión incluye reloj digital, texto y GIF. No incorpora motor de esferas de Clockwise, sensores ni una interfaz de subida de GIF integrada dentro de Home Assistant. La vista previa del navegador es aproximada.

## Licencias

Mortymel Matrix es código original MIT con una estética retro inspirada en relojes de píxeles. Clockwise pertenece a [jnthas](https://github.com/jnthas/clockwise) y no se incluyen sus archivos, fuentes ni personajes. Antes de añadir contenido ajeno, comprobar la licencia específica del recurso.
