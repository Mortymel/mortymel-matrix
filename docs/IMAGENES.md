# Fotos y GIF para Morty´s

## Qué admite el panel

| Contenido | Cómo cargarlo | Resultado |
| --- | --- | --- |
| Foto PNG, JPG/JPEG o WebP | Fotos y GIF → Foto fija | La consola la convierte localmente a 64 × 64 y 256 colores; se guarda como GIF de un solo fotograma |
| GIF animado | Fotos y GIF → Subir GIF | Conserva la animación; hasta 64 × 64 y 400 000 bytes |
| GIF estático | Subir GIF | Se muestra como imagen fija |

Las fotos de entrada admiten hasta 12 MB. El original no se envía al ESP32: la conversión se hace en tu navegador. La memoria disponible depende del perfil de placa y de los archivos ya guardados. Una foto convertida ocupa aproximadamente 5,5 KB. Los archivos permanecen tras reiniciar; una instalación USB con borrado puede eliminarlos.

## Poner una foto fija

1. Abre **Fotos y GIF → Foto fija** y selecciona un PNG, JPG o WebP.
2. Elige **Encajar completa** para conservar la imagen con márgenes negros o **Recortar al centro** para llenar el panel.
3. Revisa la miniatura y pulsa **Guardar foto**.
4. En Panel principal queda seleccionada la foto. Pulsa **Mostrar en matriz** para activarla.

El nombre interno comienza por `foto-` y termina en `.gif`; es una foto fija, no una animación. Para borrarla usa la biblioteca. La transparencia se compone sobre negro. El panel tiene pocos píxeles: favorece rostros cercanos, siluetas, símbolos y fondos sencillos; el texto pequeño y los detalles finos se perderán. La reducción a 256 colores puede producir bandas en degradados.

## Crear un GIF compatible

1. Crea un lienzo de **64 × 64 píxeles**. Los archivos menores se dibujan en la esquina superior izquierda, sin ampliación automática.
2. Usa fondo negro opaco y fotogramas completos para reducir problemas de transparencia y restos de cuadros anteriores.
3. Empieza con **10–15 fotogramas por segundo**, pocos segundos y 32–128 colores. Son recomendaciones de preparación, no requisitos estrictos.
4. Exporta como **GIF**, no MP4, APNG ni WebP animado. Evita entrelazado cuando el editor permita elegirlo.
5. Si supera 400 000 bytes, reduce duración, número de cuadros o colores. No cambies solo la extensión del archivo.
6. Sube el archivo y selecciónalo en **Panel principal → Foto o GIF → Mostrar en matriz**.

El firmware reproduce en bucle y limita la espera entre cuadros a 20–1000 ms. Los tiempos extremos o la velocidad real de decodificación pueden hacer que una animación no se vea exactamente igual que en un ordenador. La validación de subida comprueba cabecera, tamaño y dimensiones; un archivo dañado puede pasar esa comprobación sin reproducirse correctamente.

## Herramientas recomendadas

| Plataforma | Uso recomendado | Enlace |
| --- | --- | --- |
| Piskel | Dibujar pixel art y animaciones cuadro a cuadro en un lienzo pequeño | [Editor](https://www.piskelapp.com/) · [Código y documentación](https://github.com/piskelapp/piskel) |
| Ezgif | Redimensionar un GIF existente y reducir su peso | [Redimensionar](https://ezgif.com/resize) · [Optimizar](https://ezgif.com/optimize) · [Guía de optimización](https://ezgif.com/help/optimizing-gifs) |

En Piskel crea el lienzo 64 × 64, dibuja los cuadros y exporta GIF a escala 1×. En Ezgif recorta primero a un cuadrado si hace falta, redimensiona a 64 × 64 y después optimiza. Comprueba siempre las dimensiones y el peso del archivo descargado. Ezgif recibe los archivos que subas a su servicio; para fotos personales puedes usar directamente la conversión local de Morty´s.

## Entender la vista previa

La vista previa es una simulación de la escena seleccionada, no una cámara ni una captura del framebuffer. El reloj usa la hora informada por el ESP32; si aún no se sincronizó muestra `--:--`. En la demostración web los valores son ficticios.

La ampliación conserva los píxeles visibles: los bordes escalonados son normales en una matriz de baja resolución. La previsualización del reloj usa caracteres de píxel y las mismas posiciones del firmware; el texto libre es aproximado. La vista previa de un GIF representa su primer fotograma, sin estirarlo; los colores, el brillo físico y el ritmo exacto pueden variar frente al panel real.

## Si algo no se ve bien

- **Foto deformada:** usa Encajar completa; Recortar al centro conserva proporciones, pero elimina bordes.
- **GIF pequeño en una esquina:** expórtalo sobre un lienzo completo de 64 × 64.
- **Rastros o parpadeos:** usa fondo opaco y cuadros completos, y reduce la velocidad.
- **Se rechaza el archivo:** comprueba formato real, dimensiones, límite de peso y espacio libre. Los GIF deben tener nombre de hasta 36 caracteres con letras, números, guion, guion bajo o punto y extensión `.gif`.
- **No aparece en el panel:** guardar no activa una escena; pulsa Mostrar en matriz. Comprueba también la conexión HUB75 y el perfil de placa.
