# Conexión física de una matriz HUB75 64 × 64

Esta guía corresponde a **una matriz HUB75 64 × 64, barrido 1/32, con dirección E** y a un **ESP32-S3**. Mira la serigrafía y la hoja técnica de *tu panel*: existen variantes de barrido, controladores y conectores. Desconecta la fuente y el USB antes de mover cualquier cable.

## Material y alimentación

- ESP32-S3 con GPIO accesibles y flash QD compatible con el [perfil elegido](INSTALACION.md); matriz HUB75 con conector de entrada **IN**, cable HUB75/IDC correcto y una fuente **5 V externa** dimensionada para la corriente indicada por el fabricante del panel.
- Conecta la fuente 5 V al **conector de alimentación del panel**, no a un pin de señales ni a un GPIO. Conecta el GND de esa fuente al GND del panel **y** al GND del ESP32-S3: todas las señales necesitan una referencia común. Alimenta el ESP32-S3 mediante su USB o un circuito adecuado para su placa. No alimentes los LEDs desde el pin 5 V del microcontrolador.
- Recomendado: traductor lógico de 3,3 V a 5 V, como 74AHCT245, en las señales que van del ESP32 al panel. Alimenta el traductor a 5 V, comparte GND, orienta correctamente su dirección y comprueba su capacidad para las 14 señales. No apliques 5 V a un GPIO del ESP32.

## Las 14 señales y el mapa inicial

El mapa siguiente es el **predeterminado de la biblioteca ESP32-HUB75-MatrixPanel-DMA para S3**, excepto E, que Mortymel Matrix deja inicialmente en `-1` para mantener la salida apagada. Es una asignación de señales, **no** un orden físico de las patillas del conector. Conecta cada nombre del lado **IN** del panel al GPIO indicado en tu placa, directamente o mediante el traductor. Confirma en el pinout de tu placa que el GPIO esté realmente expuesto.

| Señal del panel | GPIO inicial ESP32-S3 | Función |
| --- | ---: | --- |
| R1 | 4 | Rojo, mitad superior |
| G1 | 5 | Verde, mitad superior |
| B1 | 6 | Azul, mitad superior |
| R2 | 7 | Rojo, mitad inferior |
| G2 | 15 | Verde, mitad inferior |
| B2 | 16 | Azul, mitad inferior |
| A | 18 | Dirección de filas A |
| B | 8 | Dirección de filas B |
| C | 3 | Dirección de filas C; GPIO 3 interviene en el arranque de algunas placas |
| D | 42 | Dirección de filas D |
| E | **−1; sin conectar al inicio** | Dirección de filas E. Ejemplo: GPIO 10, si existe y está libre en tu placa |
| LAT / STB | 40 | Enclavamiento |
| OE | 2 | Habilitación de salida |
| CLK | 41 | Reloj |

En un conector HUB75E típico de 2 × 8 posiciones, las **parejas de señales** por fila son estas. La tabla representa etiquetas, **no números de patilla ni orientación del cable**: la muesca, el pin 1 y la orientación de IN/OUT cambian entre paneles. Comprueba las etiquetas impresas en el tuyo antes de conectar.

| Lado de una fila | Lado opuesto |
| --- | --- |
| R1 | G1 |
| B1 | GND |
| R2 | G2 |
| B2 | E |
| A | B |
| C | D |
| CLK | LAT |
| OE | GND |

El panel también tiene un **conector de potencia independiente** para 5 V y GND. No confundas sus GND con las entradas RGB. Conecta el cable plano al conector **IN** del panel; **OUT** sirve para encadenar otra matriz y no es la entrada desde el ESP32.

## Adaptar el mapa a tu placa

1. Instala el firmware con **la matriz desconectada**. Lee los GPIO disponibles en el esquema o pinout de la placa exacta. La capacidad de flash 4/8/16 MB no determina qué pines están expuestos.
2. En la consola web local abre **Sistema → Conexión HUB75**. Ajusta las 14 señales si tu cableado requiere otros GPIO. Para los perfiles publicados se rechazan pines repetidos, GPIO 0, 19–20, 22–37, 43–46 y los GPIO inexistentes. Algunas placas reservan más pines: compruébalo tú. GPIO 3 figura por compatibilidad con el mapa inicial de la biblioteca; si afecta al arranque, cambia C a otro GPIO libre *antes de conectar el panel*.
3. Deja E en `-1`, guarda el mapa, deja reiniciar el ESP32 y confirma en **Sistema** que persiste. Después, con todo apagado, conecta las señales según el mapa guardado, las dos tierras y la fuente del panel.
4. Si todo coincide con tu matriz 1/32, asigna a E el GPIO físico elegido (por ejemplo 10 si está libre), conecta la señal E con la alimentación apagada y guarda el nuevo mapa. Al reiniciar se inicia el controlador DMA. Que la web diga “Matriz lista” solo confirma la inicialización del controlador; comprueba visualmente colores, filas y brillo.

**Evita pines comprometidos con flash, PSRAM, USB, UART y arranque.** GPIO 19/20 son USB nativo en S3; algunos GPIO dependen de la variante de memoria. Este firmware reserva conservadoramente 22–37 para no interferir con memorias de diversas placas. La interfaz no puede detectar un cable invertido, un panel de otro barrido o una fuente insuficiente.

Si la matriz queda oscura, vuelve a `E = -1` desde la web y revisa, con la alimentación quitada, fuente 5 V, GND compartido, IN, etiquetas de señales, capacidad y orden de los GPIO, barrido 1/32 y controlador. Para otras resoluciones o barridos hay que adaptar y compilar el firmware.

## Fuentes de los mapas y precauciones

- [Mapa S3 de la biblioteca HUB75 DMA](https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA/blob/cf09801a362c71fc61da99751972b6dfb6e63b63/src/platforms/esp32s3/esp32s3-default-pins.hpp) y [disposición de las señales en su ejemplo](https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA/blob/cf09801a362c71fc61da99751972b6dfb6e63b63/examples/PIO_TestPatterns/src/main.cpp).
- [Esquema y GPIO de ESP32-S3 DevKitC-1](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide.html) como ejemplo de **una** placa; consulta el esquema de la tuya.
- [Alimentación de matrices RGB](https://learn.adafruit.com/32x16-32x32-rgb-led-matrix/powering) y [adaptación de niveles HUB75](https://learn.adafruit.com/adafruit-rgb-matrix-plus-real-time-clock-hat-for-raspberry-pi/downloads) como referencias de técnica general; el consumo real depende de tu panel.
