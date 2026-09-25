# Desarrollo de Mortymel Matrix

## Hecho en este prototipo

- Consola local e instalador público en español con estética de reloj de píxeles.
- Un único firmware que incluye su página web y permite administrar GIF.
- Reloj, texto y GIF en matriz 64×64 con hardware compatible.
- Ajustes persistentes, mapa HUB75 de 14 GPIO editable y descarga OTA separada de la imagen USB.
- MQTT Discovery y entidades para escena, texto, brillo y estado.

## Siguiente etapa: compatibilidad

- Perfiles versionados para placas ESP32, S2 y S3 concretas. Una matriz 64×64
  1/32 scan exige la línea E; la compatibilidad de controladores varía.
- Tabla de particiones por flash y variante de PSRAM; aviso en la web de
  espacio libre y límites de carga. Pruebas reales de Wi-Fi con HUB75 DMA.
- Sensores opcionales como temperatura, humedad y luz con asignación de pines
  validada para cada perfil.

## Siguiente etapa: contenido

- Editor de reloj y temas dinámicos que no requieran recompilar.
- Biblioteca de GIF con listas de reproducción, horarios, prioridades de
  mensajes, vista previa de fotogramas y fallback cuando no hay red.
- Escalado y optimización de GIF en el navegador antes de enviarlos al ESP32.
- Mejoras del almacenamiento: avisos de espacio libre, integridad de GIF y recuperación de errores.

## Siguiente etapa: Home Assistant

- Panel o app dentro de Home Assistant para subir GIF desde su interfaz.
- Servicio HTTP local entre la app y el ESP32 para transferir archivos; MQTT
  para órdenes, estados y automatizaciones.
- Escenas, avisos y sensores adicionales mediante MQTT Discovery.

## Referencias y licencias

La interfaz se inspira en una estética retro, pero no reutiliza marcas, UI,
gráficos ni personajes de Clockwise. Si en el futuro se reutiliza su código,
mantener la licencia MIT original y la atribución de autor. Comprobar las
licencias de cada esfera, fuente e imagen por separado.
