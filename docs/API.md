# API web local

Todas las rutas usan HTTP Basic con usuario `admin` y la contraseña configurada al primer arranque. `GET /api/status` nunca devuelve contraseñas. La consola web y la API usan HTTP local; no abras el puerto 80 del ESP32 a Internet.

| Método | Ruta | Entrada | Resultado |
| --- | --- | --- | --- |
| GET | `/` | — | Consola web. |
| GET | `/api/status` | — | ID, Wi-Fi, MQTT, matriz, escena, brillo y ajustes no secretos. |
| POST | `/api/display` | JSON `{mode,message,gif,brightness}` | Guarda la escena. |
| POST | `/api/hardware` | JSON `{e_pin}` | Guarda el mapa de 14 señales y reinicia; acepta también `{e_pin}` de la API anterior. E `-1` desactiva el panel. |
| GET | `/api/media` | — | Lista de GIF con nombre y tamaño. |
| GET | `/api/media/<nombre.gif>` | — | GIF guardado. |
| POST | `/api/media` | `multipart/form-data`, campo `file` | Carga un GIF. |
| DELETE | `/api/media/<nombre.gif>` | — | Borra el GIF; si estaba activo, vuelve al reloj. |
| POST | `/api/network` | JSON `{ssid,password}` | Configura Wi-Fi; contraseña vacía conserva la anterior. |
| POST | `/api/mqtt` | JSON `{host,port,user,password}` | Configura broker; contraseña vacía conserva la anterior. |
| POST | `/api/time` | JSON `{timezone}` | Zona horaria POSIX, p. ej. `UTC0` o `EST5`. |
| POST | `/api/admin` | JSON `{password}` | Cambia la contraseña web, de 12 a 80 caracteres. |
| POST | `/api/update` | `multipart/form-data`, campo `file` `.bin` | Instala la **aplicación OTA**, nunca la imagen USB completa. |

Las respuestas JSON usan `application/json`; los errores tienen la forma `{"error":"..."}`. `GET /api/status` incluye `pins` (14 GPIO), `e_pin` por compatibilidad y `matrix` (si el controlador inició); ese valor no verifica visualmente el panel.

## Escenas y GIF

Para mostrar un GIF, súbelo a `/api/media` y envía `mode:"gif"` y `gif:"archivo.gif"` a `/api/display`, junto con `message` y `brightness`. Los nombres admiten letras ASCII, números, guion, guion bajo y punto, con extensión `.gif` en minúsculas. El archivo ocupa como máximo 400 KB y sus dimensiones deben estar entre 1 × 1 y 64 × 64. Se valida la cabecera y las dimensiones; la integridad de todos los fotogramas no se comprueba antes de reproducirlos.

## Ejemplo de estado

```bash
curl --user admin http://IP_DEL_ESP32/api/status
```

`curl` pedirá la contraseña si no se proporciona tras `admin`. Sustituye la IP por la mostrada en el registro serie. Los cambios de escena, GIF, red, zona horaria y MQTT persisten en la placa.

## Perfil HUB75

`POST /api/hardware` acepta `{ "pins": { "r1":4,"g1":5,"b1":6,"r2":7,"g2":15,"b2":16,"a":18,"b":8,"c":3,"d":42,"e":-1,"lat":40,"oe":2,"clk":41 } }`. Deben figurar las 14 claves, cada GPIO debe ser entero, sin repetirse y estar permitido por este firmware. `e=-1` mantiene apagada la matriz. Por compatibilidad también acepta `{ "e_pin": 10 }`, que cambia solo E. Guarda en NVS y reinicia. El mapa inicial, la alimentación y el orden físico se explican en [Cableado HUB75](CABLEADO.md).

Esta validación reserva GPIO 0, 19–20, 22–37 y 43–46. Comprueba también qué pines existen y están libres en *tu* placa; la API no descubre las conexiones reales ni el tipo de panel.

## Horario de brillo bajo

`POST /api/brightness-schedule` requiere autenticación y recibe `enabled` (booleano), `start` y `end` (HH:MM), y `brightness` (entero 1–255). Ejemplo:

```json
{"enabled":true,"start":"22:00","end":"07:00","brightness":16}
```

El intervalo incluye la hora de inicio y excluye la de fin; puede cruzar medianoche. Inicio y fin iguales se rechazan. El horario se guarda en NVS y usa la zona horaria configurada en el dispositivo. Mientras la hora no esté sincronizada no se aplica. El brillo efectivo es el menor entre el habitual y el programado: nunca se aumenta el brillo por activar el horario. Al salir del intervalo se recupera el brillo habitual, incluidos los cambios realizados por MQTT durante el horario.

`GET /api/status` añade `brightness_schedule` (enabled, start, end, brightness, active), `effective_brightness`, `time_synced` y `local_time`. El campo `brightness` continúa representando el brillo habitual.
