# API web local

Todas las rutas usan HTTP Basic: usuario `admin` y contraseña configurada
durante el primer arranque. Las respuestas JSON tienen `Content-Type:
application/json`. Los errores incluyen `{"error":"..."}`.

| Método | Ruta | Entrada | Resultado |
| --- | --- | --- | --- |
| GET | `/` | — | Consola web. |
| GET | `/api/status` | — | ID, conexión, escena y configuración sin contraseñas. |
| POST | `/api/display` | JSON `{mode,message,gif,brightness}` | Guarda y aplica escena. |
| GET | `/api/media` | — | `{"files":[{"name":"x.gif","bytes":...}]}`. |
| GET | `/api/media/<nombre.gif>` | — | Archivo GIF guardado. |
| POST | `/api/media` | `multipart/form-data`, campo `file` | Carga un GIF. |
| DELETE | `/api/media/<nombre.gif>` | — | Borra un GIF; si estaba activo, pasa a reloj. |
| POST | `/api/network` | JSON `{ssid,password}` | Configura Wi-Fi; contraseña vacía conserva la anterior. |
| POST | `/api/mqtt` | JSON `{host,port,user,password}` | Configura broker; contraseña vacía conserva la anterior. |
| POST | `/api/time` | JSON `{timezone}` | Zona horaria POSIX, p. ej. `UTC0` o `EST5`. |
| POST | `/api/admin` | JSON `{password}` | Cambia la contraseña de administración. |
| POST | `/api/update` | `multipart/form-data`, campo `file` `.bin` | Actualiza firmware y reinicia. |

Para seleccionar un GIF: cargarlo en `/api/media` y luego llamar a
`/api/display` con `mode:"gif"` y `gif:"archivo.gif"`. Los nombres admiten
letras ASCII, números, guion, guion bajo y punto; la extensión debe ser `.gif`.
El archivo debe medir entre 1×1 y 64×64 píxeles y ocupar hasta 400 KB. La
implementación valida cabecera, tamaño y dimensiones; todavía no comprueba la
integridad de todos los fotogramas antes de reproducirlos.

Ejemplo para un equipo ya configurado (la contraseña se solicita de manera
interactiva, sin incrustarla en el comando):

```bash
curl --user admin --get http://IP_DEL_ESP32/api/status
```

No exponer estas rutas en Internet. La carga OTA es una operación sensible:
usar únicamente binarios propios construidos para el modelo de hardware.

## Perfil de hardware

`POST /api/hardware` con JSON `{ "e_pin": 16 }` guarda el GPIO E y reinicia. `-1` desactiva la matriz. `GET /api/status` incluye `e_pin` y el estado de `matrix`. Las demás señales HUB75 conservan los pines predeterminados de la biblioteca. Requiere autenticación.
