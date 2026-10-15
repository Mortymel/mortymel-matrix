# Placas y perfiles compatibles

El instalador ofrece selección manual por módulo o por familia y capacidad. Hay **nueve perfiles de firmware**: ESP32 clásico, ESP32-S2 y ESP32-S3, cada uno con flash de 4, 8 o 16 MB. Los nombres de módulo son accesos al mismo perfil de familia/memoria; no cambian el cableado ni habilitan PSRAM.

| Familia | Módulos y placas incluidos en el selector |
|---|---|
| ESP32 clásico | WROOM-32/32D/32U, WROOM-32E/32UE, WROVER-E/IE, DevKitC, DevKit V1, NodeMCU-32S y PICO-D4/V3/V3-02 |
| ESP32-S2 | WROOM/WROOM-I, WROVER/WROVER-I, SOLO/SOLO-U, SOLO-2/SOLO-2U, Saola-1 y S2 Mini con S2FN4R2 |
| ESP32-S3 | WROOM-1/1U N4, N8, N16, N4R8, N8R2, N8R8 y N16R8; DevKitC-1 con flash Quad SPI |

Elige la capacidad real según la referencia impresa y las especificaciones. La misma placa comercial puede venderse con memorias diferentes. Los perfiles genéricos sirven para otras placas de estas familias cuando tengan flash SPI compatible y suficientes GPIO de salida accesibles para las 14 señales HUB75.

## Alcance de la compatibilidad

Los perfiles deben superar compilación y empaquetado en CI. Esto no equivale a probar físicamente todos los módulos, clones y placas comerciales. No hay detección automática ni cambio automático del firmware elegido. La conexión USB comprueba únicamente que la familia del chip coincida.

- La PSRAM permanece deshabilitada. Un módulo con PSRAM Octal (por ejemplo N8R8) puede tener **flash Quad SPI**: PSRAM Octal y flash Octal son cosas distintas.
- La flash Octal/OPI, por ejemplo en variantes WROOM-2, requiere otro firmware y no se ofrece aquí.
- Los chips C2/C3/C5/C6/C61/H2/P4 y otras familias no tienen un perfil en este proyecto. La biblioteca fijada soporta ESP32 clásico, S2 y S3; no basta añadir un nombre al selector para portar el controlador.
- No se ofrecen imágenes de 2 MB ni mayores de 16 MB. Las particiones OTA y LittleFS publicadas cubren 4/8/16 MB.
- El firmware conserva el soporte de panel HUB75 64×64 existente. Ampliar el catálogo de placas no añade otros controladores ni barridos.

## Cableado y actualización

Configura los GPIO desde la consola local; los nombres del selector no cargan un mapa de pines específico. Evita GPIO conectados a flash, PSRAM, USB o periféricos de la placa. En WROVER y otros módulos con PSRAM revisa especialmente GPIO 16/17; en S3 con PSRAM Octal, GPIO 35–37 también pueden estar reservados. Consulta siempre el esquema de tu placa y la [guía HUB75](CABLEADO.md).

Una placa compacta con pocos GPIO expuestos puede ejecutar el firmware sin disponer de las 14 salidas necesarias para el panel. No se puede garantizar compatibilidad de cualquier placa solo por compartir chip.

Usa OTA únicamente dentro del mismo perfil y distribución de particiones. Para pasar de 4 a 8/16 MB o cambiar de familia, respalda ajustes/GIF y reinstala por USB: el instalador borra la flash completa.

## Referencias

- [Biblioteca HUB75 fijada por el proyecto](https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA/tree/cf09801a362c71fc61da99751972b6dfb6e63b63).
- [ESP32-S3-WROOM-1/1U: flash y PSRAM](https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf).
- [ESP32-WROOM-32E/32UE](https://documentation.espressif.com/esp32-wroom-32e_esp32-wroom-32ue_datasheet_en.pdf).
- [ESP32-S2-SOLO/SOLO-U](https://documentation.espressif.com/esp32-s2-solo_esp32-s2-solo-u_datasheet_en.html).
- [ESP32-PICO](https://documentation.espressif.com/esp32-pico_series_datasheet_en.html).
