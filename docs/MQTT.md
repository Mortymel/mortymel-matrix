# MQTT y Home Assistant

La raíz de temas de cada dispositivo es `mortymel/matrix_XXXXXX`, donde
`XXXXXX` procede de parte de la dirección MAC del ESP32. La web muestra el ID
en **Sistema**. El ESP32 publica configuraciones de descubrimiento retenidas
en `homeassistant/<tipo>/<id>/<nombre>/config`.

| Entidad en Home Assistant | Comando | Estado | Valores |
| --- | --- | --- | --- |
| `select` Escena | `<raíz>/set/mode` | `<raíz>/state/mode` | `clock`, `text`, `gif`. |
| `text` Mensaje | `<raíz>/set/text` | `<raíz>/state/text` | Texto de hasta 80 caracteres. |
| `number` Brillo | `<raíz>/set/brightness` | `<raíz>/state/brightness` | Enteros de 1 a 255. |
| `sensor` Estado | — | `<raíz>/state/status` | `ready` o `no_panel_profile`. |

El tema retenido `<raíz>/availability` envía `online` y tiene Last Will
`offline`. Los comandos se aplican en el ESP32 y se devuelven al tema de
estado. Al reconectar, se repiten descubrimiento y estados.

**Media:** MQTT no transporta los GIF. Se suben por HTTP en la web del ESP32,
y su nombre se selecciona allí. Está previsto crear un panel o app de Home
Assistant para cargar archivos sin salir de su interfaz. El control MQTT
actual permite elegir la escena GIF, pero la elección de un GIF concreto se
hace en la web del dispositivo.

### Ejemplo de automatización

Sustituir el ID por el de la instalación:

```yaml
alias: Mostrar alerta en Mortymel Matrix
triggers:
  - trigger: state
    entity_id: binary_sensor.puerta
    to: "on"
actions:
  - action: mqtt.publish
    data:
      topic: mortymel/matrix_XXXXXX/set/text
      payload: "PUERTA ABIERTA"
  - action: mqtt.publish
    data:
      topic: mortymel/matrix_XXXXXX/set/mode
      payload: text
```

El broker necesita autenticación o una red local aislada. Este prototipo usa
MQTT TCP sin TLS; no publicar el puerto 1883 a Internet.
