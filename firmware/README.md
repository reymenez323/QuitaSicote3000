# Firmware

Dos proyectos **independientes** (cada uno con su `platformio.ini`, a crear al empezar a programar):

| Carpeta | MCU | Rol |
|---|---|---|
| [control/](control/) | Arduino Mega 2560 | Proceso, sensado ambiental, actuadores |
| [sis/](sis/) | Arduino Nano ([ADR-0002](../docs/decisiones/ADR-0002-mcu-del-sis.md)) | Seguridad |
| [hmi/](hmi/) | ESP32-32E con pantalla 3.2" ([ADR-0005](../docs/decisiones/ADR-0005-hmi-esp32.md)) | UI y registro; sólo comunicación |
| [compartido/](compartido/) | — | Contrato entre ambos (cabeceras, constantes, protocolo) |

Estructura interna de cada proyecto:

```
src/      Código fuente
include/  Cabeceras públicas del proyecto
lib/      Bibliotecas locales (drivers propios)
test/     Pruebas (nativas en PC con PlatformIO "native" donde sea posible)
```

Reglas:

- `sis/` **no** incluye código de `control/`. Sólo puede incluir `compartido/protocolo/`.
- `compartido/` no contiene lógica, sólo definiciones.
- El firmware del SIS tiene versión propia; cada cambio requiere repetir la validación de [pruebas-de-validacion-sis.md](../docs/03-seguridad/pruebas-de-validacion-sis.md).
