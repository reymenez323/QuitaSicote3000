# Guía de conexión paso a paso

Esta guía explica cómo conectar **todos** los dispositivos del QuitaSicote3000. Está escrita para una persona que lo hace por primera vez y no conoce el proyecto: no hace falta haber leído nada más. Si algo no está claro, está mal escrito y hay que corregir esta guía.

**Cómo usarla**: sigue los pasos en orden, sin saltarte ninguno, y marca cada casilla ☐ cuando termines. Cada paso tiene una tabla "Desde → Hacia" con los cables y, al final, una comprobación.

> **Fuentes**: esta guía resume el [diagrama de Fritzing](../../hardware/fritzing/) y los documentos de pinout ([control](../pinout/control-esp32.md), [SIS](../pinout/sis-esp32.md), [HMI](../pinout/esp32-hmi.md)). Si alguna vez la guía y los pinout no coinciden, **mandan los pinout** y hay que avisar para corregir la guía.

---

## Contenido

1. [Qué estás construyendo](#1-qué-estás-construyendo)
2. [Seguridad: léelo antes de tocar nada](#2-seguridad-léelo-antes-de-tocar-nada)
3. [Materiales y herramientas](#3-materiales-y-herramientas)
4. [El diagrama completo](#4-el-diagrama-completo)
5. [Preparación](#5-preparación)
6. [Montaje paso a paso](#6-montaje-paso-a-paso)
7. [Revisión con el equipo apagado](#7-revisión-con-el-equipo-apagado)
8. [Primer encendido por etapas](#8-primer-encendido-por-etapas)
9. [Si algo no funciona](#9-si-algo-no-funciona)
10. [Puntos que aún hay que verificar](#10-puntos-que-aún-hay-que-verificar)

---

## 1. Qué estás construyendo

Un equipo doméstico que quita el mal olor del calzado con **aire caliente**. Un calefactor (el **PTC**, 100 W) y varios ventiladores mueven el aire por una cámara; el usuario lo maneja sólo desde una **pantalla táctil**.

### Las tres placas

Hay **tres placas con microcontrolador ESP32**. Cada una tiene un trabajo distinto y **no se deben mezclar**:

| Nombre | Placa | Qué hace | Cómo reconocerla |
|---|---|---|---|
| **CONTROL** | ESP32 DevKit V1 (30 pines) | Es el "cerebro": lee temperatura, humedad y olor; enciende el calefactor y los ventiladores; ejecuta el ciclo | Placa de desarrollo negra, con dos filas de pines |
| **SIS** (sistema de seguridad) | ESP32 DevKit V1 (30 pines), **idéntica** a la anterior | Es el "vigilante": puede **cortar el calefactor** por su cuenta si algo va mal, aunque el control falle | Igual que CONTROL |
| **HMI** (pantalla) | ESP32-32E con pantalla táctil de 3,2" | Sólo muestra información y envía las órdenes del usuario | Pantalla con marco azul |

> ⚠️ **CONTROL y SIS son la misma placa.** Pon una etiqueta ("CONTROL" y "SIS") **antes de cablear nada**. Si las confundes, el equipo no será seguro.

### Las tensiones

| Tensión | Dónde está | Para qué |
|---|---|---|
| **120 V de corriente alterna** | Sólo dentro de la fuente de alimentación | Entra a la fuente. **Es peligrosa** |
| **12 V continua** | Fuente → calefactor, ventiladores, bobinas de los relés, reguladores | Potencia |
| **5 V continua** | Salida de los dos reguladores | Alimenta las placas ESP32 y la pantalla |
| **3,3 V** | Pin `3V3` de la placa CONTROL | Alimenta los sensores. **Todas las señales entre placas son de 3,3 V** |

> ⚠️ **Ningún pin de los ESP32 tolera 5 V ni 12 V.** Un cable de 12 V o 5 V en un pin de señal (los de color azul en el diagrama) destruye la placa. Revisa dos veces cada cable antes de conectarlo.

### Cómo se reparte la seguridad

El equipo corta el calefactor por **tres vías independientes**. Por eso hay cosas que parecen "repetidas":

1. El **SSR1** (lo maneja CONTROL) enciende y apaga el calefactor.
2. El relé **RL1** (lo maneja SIS) está **en serie** con el calefactor: si SIS lo abre, el calefactor se apaga aunque SSR1 falle.
3. El **termostato TH1** (80 °C) también está en serie: abre solo si el aire se calienta demasiado.

Por eso el **orden de los componentes de la rama del calefactor importa** (ver [paso 9](#paso-9--rama-del-calefactor-ptc)).

---

## 2. Seguridad: léelo antes de tocar nada

- **Trabaja siempre con el equipo desenchufado** de la pared. Para meter o quitar cualquier cable, la fuente tiene que estar apagada y desconectada.
- **120 V CA**: sólo entra por la fuente de alimentación (bornes `L`, `N` y tierra ⏚). **Esa conexión no está dibujada en el diagrama.** Hazla **al final**, con el cable de alimentación con clavija y tierra, y con los bornes tapados. **Si no tienes experiencia con corriente alterna, pide a un electricista que haga esa conexión.**
- **No hay fusibles** (decisión del proyecto). Por eso:
  - El cable grueso (**12 AWG / 4 mm²**) es obligatorio en la rama del calefactor y en el cable principal de 12 V. El calefactor consume unos **8,3 A** (más en el arranque): un cable fino se calienta e incendia.
  - Los cables finos (ventiladores, módulos) deben ser **cortos, sujetos y lejos de material inflamable**.
  - Un cortocircuito en un cable fino **no está protegido**. Mide siempre con el multímetro antes de encender.
- **Polaridad**: rojo = +12 V, negro = tierra. Invertirla destruye módulos al instante.
- **El calefactor** se calienta mucho. Ponlo sobre una superficie no inflamable, nunca lo pruebes sin vigilancia y ten un extintor cerca en las pruebas con el calefactor real.
- **No conectes el cable USB de una placa mientras su regulador de 5 V esté conectado**, salvo que se haya verificado que la placa lo permite (ver [§10](#10-puntos-que-aún-hay-que-verificar)). Si no, puedes dañar la placa o el ordenador.
- Si algo huele a quemado, hace chispas o se calienta mucho: **desconecta la fuente de la pared** y revisa.

---

## 3. Materiales y herramientas

### Herramientas

- ☐ **Multímetro** con prueba de continuidad (el que "pita") y medición de tensión continua. **Imprescindible.**
- ☐ Pelacables, crimpadora para terminales y destornilladores pequeños (los bornes de tornillo de los módulos).
- ☐ Etiquetas o cinta de papel y un marcador.
- ☐ Cinta aislante o termorretráctil.
- ☐ Ordenador con **PlatformIO** y cables USB (uno por placa) para cargar el firmware ([entorno](../../firmware/README.md)).
- ☐ Opcional pero recomendable: una **lámpara de 12 V / 21 W** (de auto) o una resistencia de potencia, para probar **sin el calefactor real** (ver [§8](#8-primer-encendido-por-etapas)).

### Componentes

| Ref. | Qué es | Cantidad | Nota |
|---|---|---|---|
| PSU | Fuente de **12 V DC / 20 A** | 1 | Entrada 120 V CA |
| TB12 | Barra o regleta de bornes para **+12 V** | 1 | Con bornes suficientes (≥ 14) |
| TBG | Barra o regleta de bornes para **GND** (tierra en estrella) | 1 | Con bornes suficientes (≥ 20) |
| REG_A | Regulador *buck* 12 → 5 V, **≥ 2 A** | 1 | Alimenta CONTROL y HMI |
| REG_B | Regulador *buck* 12 → 5 V, **≥ 1 A** | 1 | Alimenta SIS |
| CONTROL | ESP32 DevKit V1 | 1 | Etiquetar |
| SIS | ESP32 DevKit V1 | 1 | Etiquetar |
| HMI | ESP32-32E con pantalla 3,2" | 1 | Con su conector I2C de 4 pines (1,25 mm) |
| M1, M2 | Módulo MAX6675 (para termopar K) | 2 | |
| TC1, TC2 | Termopar tipo K, **unión aislada** | 2 | |
| S1 | Módulo SHT31 (temperatura y humedad) | 1 | |
| S2 | Módulo SGP40 (calidad del aire / olor) | 1 | |
| SW1 | Interruptor de puerta (*limit switch*) SPDT: COM, NA, NC | 1 | |
| RL1 | Módulo de relé **30 A**, disparo **ALTO** (activo con 3,3 V) | 1 | Los de 10 A **no sirven** |
| RL2 | Módulo de relé de 1 canal, disparo ALTO | 1 | |
| RM1 | Módulo de **2 relés**, disparo ALTO (RL3 y RL4) | 1 | |
| SSR1 | Relé de estado sólido DC-DC, **≥ 40 A**, con disipador | 1 | Para el calefactor |
| SSR2 | Relé de estado sólido DC-DC | 1 | Para el ventilador de circulación |
| TH1 | Termostato bimetálico **80 °C**, normalmente cerrado, rearme automático | 1 | 2 cables |
| PTC | Calefactor PTC 100 W / 12 V | 1 | |
| FAN_P | Ventilador de 12 V (el del calefactor) | 1 | |
| FAN_C | Ventilador de 12 V (circulación) | 1 | |
| FAN_B | Ventilador de 12 V (enfría la cámara de circuitos) | 1 | |

> Los tres relés "RL" y el módulo RM1 deben **activarse con 3,3 V** y quedar **desactivados con la entrada sin conectar**. Si un módulo no cumple eso, cámbialo (ver [§8](#8-primer-encendido-por-etapas)).

### Cables

| Uso | Calibre | Color sugerido |
|---|---|---|
| Cable principal de 12 V y **toda la rama del calefactor** (fuente → TH1 → RL1 → PTC → SSR1 → tierra) | **12 AWG (4 mm²)** | Rojo / negro |
| Ventiladores, bobinas de relés, reguladores | 20–22 AWG | Rojo (+12 V) / negro (GND) |
| 5 V (salida de los reguladores) | 20–22 AWG | Naranja |
| Señales de 3,3 V (GPIO, I2C, SPI, UART, puerta) | 24–26 AWG (tipo *jumper*) | Azul |
| Termopares | El cable propio del termopar (no lo cortes; trenzado) | — |

Usa **terminales** (de pala, de aguja) en los bornes de tornillo y **terminales de ≥ 25 A** en la rama del calefactor.

---

## 4. El diagrama completo

Este es el diagrama de Fritzing con **todas** las conexiones. Está en alta resolución (7740 × 5829 px): ábrelo en el archivo original para acercarte a cualquier cable.

[![Diagrama completo de conexiones](../../hardware/fritzing/QuitaSicote3000_conexiones_bb.png)](../../hardware/fritzing/QuitaSicote3000_conexiones_bb.png)

Archivos:

| Archivo | Para qué |
|---|---|
| [QuitaSicote3000_conexiones_bb.png](../../hardware/fritzing/QuitaSicote3000_conexiones_bb.png) | Imagen completa, 7740 × 5829 px |
| [QuitaSicote3000_conexiones_preview.png](../../hardware/fritzing/QuitaSicote3000_conexiones_preview.png) | Versión más ligera, 2140 × 1560 px |
| [QuitaSicote3000_conexiones.fzz](../../hardware/fritzing/QuitaSicote3000_conexiones.fzz) | Archivo editable de [Fritzing](https://fritzing.org/) |

### Código de colores de los cables

| Color | Significa |
|---|---|
| 🟥 **Rojo** | +12 V (incluye tramos conmutados de 12 V) |
| 🟥 **Rojo con bandas blancas** | Retorno de carga hacia el SSR o el relé (**no es tierra**, aunque lleva 12 V) |
| 🟧 **Naranja** | 5 V (salida de los reguladores) |
| 🟧 **Naranja con bandas** | 3,3 V del pin `3V3` de CONTROL (sensores) |
| 🟦 **Azul** | Señales (GPIO, SPI, I2C, UART, puerta, termopares) |
| ⬛ **Negro** | Tierra (GND) |

### El diagrama por zonas

Los recortes de abajo son partes del diagrama a escala completa. Úsalos junto a cada paso.

| Zona | Qué contiene | Imagen |
|---|---|---|
| **1** | Fuente, barras de 12 V y GND, reguladores y pantalla | [zona1-fuente-y-reguladores.png](zona1-fuente-y-reguladores.png) |
| **2** | Placa CONTROL y sensores | [zona2-control-y-sensores.png](zona2-control-y-sensores.png) |
| **3** | Placa SIS y puerta | [zona3-sis-y-puerta.png](zona3-sis-y-puerta.png) |
| **4** | Relés, calefactor, SSR, ventiladores y termostato | [zona4-cargas.png](zona4-cargas.png) |

> ℹ️ **En el diagrama, la placa SIS está dibujada girada 180°** (con el conector USB a la izquierda) para que los cables no se crucen. En tu placa real los pines tienen las mismas etiquetas: **guíate siempre por la etiqueta impresa (`D25`, `GND`, `VIN`…), no por la posición en el dibujo.**

### Cómo leer las etiquetas de los pines

En la placa ESP32 DevKit V1 los pines dicen `D25`, `D26`… eso es el **GPIO** del mismo número (`D25` = GPIO25). Algunos tienen otro nombre:

| En la placa dice | En esta guía |
|---|---|
| `D16` / `RX2` | GPIO16 (RX del UART2) |
| `D17` / `TX2` | GPIO17 (TX del UART2) |
| `3V3` | 3,3 V de salida |
| `VIN` | Entrada de 5 V |
| `GND` | Tierra (hay varios, todos son el mismo) |

---

## 5. Preparación

### ☐ 5.1 Etiqueta todo

- Etiqueta las dos placas ESP32: **CONTROL** y **SIS**.
- Etiqueta cada módulo con su referencia (RL1, RL2, RM1, SSR1, SSR2, M1, M2, S1, S2, REG_A, REG_B).
- Etiqueta los bornes de las barras TB12 y TBG si puedes.

### ☐ 5.2 Carga el firmware en cada placa (antes de cablear)

Con cada placa **sola**, conectada por USB al ordenador y **sin ningún otro cable**:

| Placa | Carpeta del firmware |
|---|---|
| CONTROL | [firmware/control](../../firmware/control/) |
| SIS | [firmware/sis](../../firmware/sis/) |
| HMI | [firmware/hmi](../../firmware/hmi/) |

Ver [firmware/README.md](../../firmware/README.md) para las instrucciones de PlatformIO. Cargar el firmware ahora evita tener que conectar el USB después, con los reguladores puestos.

> Al **primer encendido** de la pantalla HMI te pedirá **calibrar el táctil**: toca con el dedo las cuatro esquinas marcadas. Se guarda; para repetirlo, mantén el dedo en la pantalla 3 s al encender.

### ☐ 5.3 Ajusta los reguladores a 5,0 V (antes de conectar ninguna placa)

Los reguladores *buck* suelen venir ajustados a otro valor. **Si conectas una placa con una tensión más alta, la dañas.**

1. Conecta **sólo** el regulador a la fuente de 12 V (entrada `IN+` / `IN−`), sin nada en la salida.
2. Pon el multímetro en tensión continua entre `OUT+` y `OUT−`.
3. Gira el tornillo de ajuste (o el potenciómetro azul) hasta leer **5,0 V a 5,1 V**.
4. Anota el valor y marca el regulador ("5V OK").

Hazlo para **REG_A y REG_B**. Es más cómodo hacerlo cuando ya tengas el +12 V en la barra TB12 ([paso 1](#paso-1--fuente-y-barras-de-12-v-y-tierra)): puedes dejar esta tarea para el [paso 2](#paso-2--reguladores-de-5-v).

### ☐ 5.4 Comprueba cada módulo de relé y cada SSR "en frío"

Antes de montarlos, comprueba con una fuente de 12 V (o la propia fuente) que:

- cada módulo de relé **hace clic** al darle 3,3 V en `IN` (por ejemplo desde el pin `3V3` de una placa ESP32), y
- **no hace clic** con la entrada sin conectar.

Si un módulo se comporta al revés, **no lo uses**: puede dejar el calefactor o un ventilador en estado erróneo al arrancar.

---

## 6. Montaje paso a paso

> **Regla durante todo el montaje**: la fuente está **desenchufada de la pared**.
> **Orden**: primero tierra y 12 V, luego reguladores, luego las placas y sus sensores, luego los módulos de potencia, y **al final el calefactor**.

### Paso 1 — Fuente y barras de 12 V y tierra

📷 Zona 1: [zona1-fuente-y-reguladores.png](zona1-fuente-y-reguladores.png)

La fuente tiene varios bornes `V+` y varios `V−` (todos iguales entre sí). Las dos barras de bornes (TB12 y TBG) reparten la energía a todo lo demás. La barra de GND es la **"estrella de tierra"**: **todos** los negativos terminan ahí, cada uno con su cable.

| Desde | Hacia | Cable |
|---|---|---|
| Fuente `V+` | Barra **TB12** (+12 V) | Rojo, **12 AWG** |
| Fuente `V−` | Barra **TBG** (GND) | Negro, **12 AWG** |

☐ Comprobación (multímetro en continuidad, fuente apagada y desenchufada): entre la barra TB12 y la barra TBG **no** debe haber continuidad (no debe pitar). Si pita, hay un cortocircuito: busca el cable que toca.

### Paso 2 — Reguladores de 5 V

📷 Zona 1.

Cada regulador baja 12 V a 5 V. **REG_A** alimenta CONTROL y la pantalla HMI. **REG_B** alimenta sólo SIS: son independientes a propósito, para que si uno falla el otro siga.

| Desde | Hacia | Cable |
|---|---|---|
| TB12 (+12 V) | REG_A `IN+` | Rojo |
| TBG (GND) | REG_A `IN−` | Negro |
| TB12 (+12 V) | REG_B `IN+` | Rojo |
| TBG (GND) | REG_B `IN−` | Negro |

Cada regulador debe tener **su propio cable de tierra** hasta TBG (no compartas cables de tierra).

☐ Ajusta REG_A y REG_B a 5,0–5,1 V ([§5.3](#-53-ajusta-los-reguladores-a-50-v-antes-de-conectar-ninguna-placa)). **Todavía no conectes la salida de los reguladores a ninguna placa.**

### Paso 3 — Placa CONTROL: sensores

📷 Zona 2: [zona2-control-y-sensores.png](zona2-control-y-sensores.png)

La placa CONTROL lee cuatro sensores. Todos funcionan a **3,3 V**, que sale del pin `3V3` de la propia placa CONTROL.

#### 3A — Alimentación de los sensores

| Desde | Hacia | Cable |
|---|---|---|
| CONTROL `3V3` | M1 `VCC`, M2 `VCC`, S1 `VIN`, S2 `VIN` | Naranja con bandas / 3,3 V |
| CONTROL `GND` | M1 `GND`, M2 `GND`, S1 `GND`, S2 `GND` | Negro |

> ⚠️ **Los sensores van a 3,3 V, NO a 5 V.** Si los alimentas con 5 V, sus señales de salida llegarían a 5 V y dañarían la placa CONTROL.

#### 3B — Termopares (temperatura del aire)

Cada termopar se conecta a un módulo MAX6675 (M1 y M2), que se comunica con CONTROL por SPI.

| Desde | Hacia | Cable |
|---|---|---|
| TC1 (aire de la recámara) | M1, bornes `+` y `−` | Cable del propio termopar |
| TC2 (aire a la salida del calefactor) | M2, bornes `+` y `−` | Cable del propio termopar |

- Respeta la polaridad: el hilo **positivo** del termopar al borne `+`. En el estándar ANSI de termopar K el cable positivo suele ser **amarillo** y el negativo **rojo**; confírmalo en la hoja del termopar. Si al calentar la lectura baja, están invertidos.
- Usa termopares de **unión aislada** (la punta no debe tocar metal conectado a tierra).
- No cortes ni alargues el cable de forma improvisada; mantenlo **lejos de los cables de potencia**.

Señales entre CONTROL y los MAX6675:

| CONTROL (pin) | M1 | M2 | Qué es |
|---|---|---|---|
| `D18` | `SCK` | `SCK` | Reloj SPI (compartido) |
| `D19` | `SO` | `SO` | Datos SPI (compartido) |
| `D23` | `CS` | — | Selección de M1 |
| `D13` | — | `CS` | Selección de M2 |

(No se conecta ningún pin `MOSI`: el MAX6675 sólo envía datos.)

#### 3C — SHT31 y SGP40 (humedad y olor)

Ambos van por I2C, que usa **dos cables compartidos** por los dos sensores.

| CONTROL (pin) | S1 (SHT31) | S2 (SGP40) | Qué es |
|---|---|---|---|
| `D21` | `SDA` | `SDA` | Datos I2C (compartido) |
| `D22` | `SCL` | `SCL` | Reloj I2C (compartido) |

En el SHT31 deja los pines `ADR` y `AL` **sin conectar**.

☐ Comprobación: con el multímetro en continuidad, verifica que **`3V3` y `GND` no hacen continuidad** entre sí (en la placa CONTROL, sin alimentar).

### Paso 4 — Interruptor de la puerta (SW1)

📷 Zona 3: [zona3-sis-y-puerta.png](zona3-sis-y-puerta.png)

El interruptor de la puerta se conecta **a las dos placas a la vez** (CONTROL y SIS), para que las dos sepan si la puerta está abierta. Abrir la puerta corta el calefactor.

| Desde | Hacia | Cable |
|---|---|---|
| SW1 `COM` | TBG (GND) | Negro |
| SW1 `NA` | CONTROL `D32` **y** SIS `D32` (los dos pines juntos) | Azul |
| SW1 `NC` | CONTROL `D33` **y** SIS `D33` (los dos pines juntos) | Azul |

- "Los dos pines juntos" significa que desde el `NA` del interruptor sale un cable que se **divide** en dos (un cable a cada placa). Igual con `NC`.
- Con la puerta cerrada (actuador presionado): `NA` conectado a `COM` y `NC` desconectado.
- Los pines `D32` y `D33` ya tienen resistencia interna: **no pongas resistencias externas**.

### Paso 5 — Pantalla HMI y enlace con CONTROL

📷 Zona 1 (pantalla) y zona 2 (CONTROL).

La pantalla recibe 5 V por su conector **USB-C** (se puede usar el cable USB-C cortado, o un conector de 5 V) y se comunica con CONTROL por **dos cables de señal** y la tierra.

#### 5A — Alimentación de CONTROL y de la pantalla

REG_A alimenta a las dos placas. **Hazlo después de ajustar REG_A a 5 V** ([§5.3](#-53-ajusta-los-reguladores-a-50-v-antes-de-conectar-ninguna-placa)).

| Desde | Hacia | Cable |
|---|---|---|
| REG_A `OUT+` (5 V) | CONTROL `VIN` | Naranja |
| REG_A `OUT−` | CONTROL `GND` | Negro |
| REG_A `OUT+` (5 V) | HMI USB-C `5V` (VBUS) | Naranja |
| REG_A `OUT−` | HMI USB-C `GND` | Negro |

> ⚠️ Con el cable USB del ordenador conectado a CONTROL o a la pantalla, **no** conectes REG_A.

#### 5B — Enlace de datos (conector I2C de 4 pines de la pantalla)

La pantalla tiene un conector pequeño de 4 pines etiquetado **I2C**. Aquí **no se usa para I2C**: se aprovechan sus pines `IO25` e `IO32` como enlace serie con CONTROL.

| Pantalla (conector de 4 pines) | CONTROL | Qué es |
|---|---|---|
| `IO25` (TX2) | `D35` | La pantalla **envía** → CONTROL **recibe** |
| `IO32` (RX2) | `D4` | CONTROL **envía** → la pantalla **recibe** |
| `GND` | `GND` | Tierra común |
| `3V3` | **NO conectar** | — |

> ⚠️ **Los cables van cruzados**: el "enviar" (TX) de uno va al "recibir" (RX) del otro.
> ⚠️ **Verifica el orden de los 4 pines del conector con el multímetro** ([§10](#10-puntos-que-aún-hay-que-verificar)) antes de conectar, y **no conectes el pin 3V3 del conector**.

### Paso 6 — Placa SIS

📷 Zona 3: [zona3-sis-y-puerta.png](zona3-sis-y-puerta.png)

El SIS **no tiene sensores propios**: sólo la puerta (paso 4), el enlace con CONTROL y las señales a sus relés. Se alimenta de **REG_B**.

| Desde | Hacia | Cable |
|---|---|---|
| REG_B `OUT−` | SIS `GND` | Negro |
| REG_B `OUT+` (5 V) | SIS `VIN` | Naranja |

> ⚠️ Antes de unir `OUT+` a `VIN`, comprueba con el multímetro que REG_B da 5,0–5,1 V.

#### Enlace entre SIS y CONTROL (UART2)

| SIS | CONTROL | Qué es |
|---|---|---|
| `D17` (TX2) | `D16` (RX2) | SIS envía → CONTROL recibe |
| `D16` (RX2) | `D17` (TX2) | CONTROL envía → SIS recibe |
| `GND` | `GND` | Tierra común |

Cruzados otra vez: TX con RX. La **tierra común** entre las tres placas es obligatoria: sin ella no hay comunicación fiable.

### Paso 7 — Módulos de relé y SSR: alimentación y señales

📷 Zona 4: [zona4-cargas.png](zona4-cargas.png)

Cada módulo tiene dos "lados":

- **Lado de control** (bobina o entrada): lo activan las placas con una señal de 3,3 V. Los módulos de relé también necesitan **12 V** para su bobina.
- **Lado de potencia** (contactos o salida): conecta o corta los 12 V de una carga. **Estos cables se hacen en los pasos 8 y 9.**

#### 7A — Alimentación de las bobinas (12 V)

| Módulo | `DC+` ← | `DC−` → |
|---|---|---|
| RL1 | TB12 (+12 V) | TBG (GND) |
| RL2 | TB12 (+12 V) | TBG (GND) |
| RM1 | TB12 (+12 V) | TBG (GND) |

> Algunos módulos de 2 relés se alimentan a 5 V. El del diagrama usa 12 V: **comprueba la etiqueta de tu módulo**.

#### 7B — Señales de CONTROL (a sus módulos)

| CONTROL (pin) | Destino | Qué hace |
|---|---|---|
| `D25` | **SSR1** `3 IN+` | ALTO = calentar |
| `D26` | **SSR2** `3 IN+` | ALTO = encender ventilador de circulación |
| `D27` | **RL2** `IN` | ALTO = **pedir apagar** el ventilador del calefactor |

| Desde | Hacia |
|---|---|
| SSR1 `4 IN−` | TBG (GND) |
| SSR2 `4 IN−` | TBG (GND) |

#### 7C — Señales de SIS (a sus módulos)

| SIS (pin) | Destino | Qué hace |
|---|---|---|
| `D25` | **RL1** `IN` | ALTO = **permiso para calentar** (relé cerrado) |
| `D26` | **RM1** `IN1` (relé RL3) | ALTO = **permitir** apagar el ventilador del calefactor |
| `D27` | **RM1** `IN2` (relé RL4) | ALTO = **forzar** el ventilador de circulación |

> ⚠️ Estos cables son de **3,3 V de señal**. Nunca conectes aquí +12 V ni +5 V.

☐ Comprobación: ningún cable de 12 V (rojo) toca un pin de las placas ESP32.

### Paso 8 — Ventiladores y sus relés

📷 Zona 4.

Hay tres ventiladores. **FAN_B** siempre gira; los otros dos los gobiernan, **juntos**, CONTROL y SIS.

#### 8A — FAN_B (cámara de circuitos): siempre encendido

| Desde | Hacia | Cable |
|---|---|---|
| TB12 (+12 V) | FAN_B `+` | Rojo |
| FAN_B `−` | TBG (GND) | Negro |

#### 8B — FAN_P (ventilador del calefactor)

Gira siempre, **salvo** que **CONTROL pida apagarlo y SIS lo permita**. Por eso hay dos contactos **NC** (normalmente cerrados) **en paralelo**: el ventilador sólo se apaga si **ambos** relés (RL2 y RL3) se activan a la vez. Si cualquiera de las placas falla o está apagada, el ventilador **queda encendido** (lo seguro).

| Desde | Hacia | Cable |
|---|---|---|
| TB12 (+12 V) | RL2 `COM` | Rojo |
| TB12 (+12 V) | RM1 `COM1` (relé RL3) | Rojo |
| RL2 `NC` | FAN_P `+` | Rojo |
| RM1 `NC1` | FAN_P `+` (el mismo punto que el anterior) | Rojo |
| FAN_P `−` | TBG (GND) | Negro |

#### 8C — FAN_C (ventilador de circulación)

Se enciende si **CONTROL lo pide (SSR2) o SIS lo fuerza (RL4)**: salidas **en paralelo**.

| Desde | Hacia | Cable |
|---|---|---|
| TB12 (+12 V) | FAN_C `+` | Rojo |
| FAN_C `−` | SSR2 `2 OUT+` | Rojo con bandas |
| FAN_C `−` | RM1 `COM2` (relé RL4) | Rojo con bandas (el mismo punto) |
| SSR2 `1 OUT−` | TBG (GND) | Negro |
| RM1 `NA2` | TBG (GND) | Negro |

> ℹ️ El cable "rojo con bandas" lleva el **negativo del ventilador**, que no está en tierra: sólo se une a tierra cuando se activa SSR2 o RL4. Por eso **no es un cable negro**.

### Paso 9 — Rama del calefactor (PTC)

📷 Zona 4.

> ⚠️ **Esta es la parte más peligrosa. Todos los cables de este paso son de 12 AWG (4 mm²), con terminales firmes y bien apretados.** Corriente aproximada: 8,3 A en régimen (más en el arranque). **Un mal contacto aquí se calienta.**
> ⚠️ **Hazlo al final y, para las primeras pruebas, sustituye el PTC por una lámpara de 12 V / 21 W** ([§8](#8-primer-encendido-por-etapas)).

El calefactor recibe 12 V a través de **tres "interruptores" en serie**: cualquiera que se abra apaga el calefactor.

```
TB12 (+12 V) ─► TH1 ─► RL1 (COM → NA) ─► PTC (+)    PTC (−) ─► SSR1 (2 OUT+)    SSR1 (1 OUT−) ─► TBG (GND)
                termostato  relé del SIS                           relé de estado sólido (CONTROL)
```

| Desde | Hacia | Cable |
|---|---|---|
| TB12 (+12 V) | TH1 (cualquiera de sus 2 cables) | Rojo, 12 AWG |
| TH1 (el otro cable) | RL1 `COM` | Rojo, 12 AWG |
| RL1 `NA` | PTC `+` | Rojo, 12 AWG |
| PTC `−` | SSR1 `2 OUT+` | Rojo con bandas, 12 AWG |
| SSR1 `1 OUT−` | TBG (GND) | Negro, 12 AWG |

- **RL1 `NC` se deja sin conectar.** (Dibujado en rojo-tachado en el diagrama: sin uso.)
- **TH1** se monta en el aire caliente, cerca de la salida del calefactor, **sin tocar las aletas** del PTC.
- Pon un **disipador** al SSR1 y no lo encierres sin ventilación.

☐ Comprobación con la fuente apagada: entre `PTC +` y `PTC −` **no** debe haber continuidad directa (no se debe puenteear el calefactor).

---

## 7. Revisión con el equipo apagado

☐ Repasa el montaje completo con el multímetro **antes de enchufar la fuente**. Con la fuente **desenchufada**:

| Comprobación | Resultado esperado |
|---|---|
| Continuidad entre TB12 y TBG | **No** pita (no hay cortocircuito) |
| Continuidad entre `3V3` y `GND` de CONTROL | **No** pita |
| Continuidad entre la salida `+` y `−` de REG_A y REG_B | **No** pita |
| Continuidad de cada `GND` de las placas, módulos y ventiladores con TBG | **Sí** pita |
| El pin `D25`, `D26`, `D27` de CONTROL y SIS con TB12 (+12 V) | **No** pita (nada de 12 V en pines de señal) |
| `D32` y `D33` de CONTROL con `D32` y `D33` de SIS | **Sí** pita, respectivamente (comparten el cable de la puerta) |
| `TX` de una placa con el `RX` de la otra (los tres enlaces) | **Sí** pita (cruzados); `TX` con `TX`: **no** |
| Todos los cables rojos gruesos del [paso 9](#paso-9--rama-del-calefactor-ptc) | Tornillos apretados; se tira suavemente de cada cable y no se mueve |

Revisa visualmente que no haya hilos sueltos asomando de los terminales y que ningún cable fino toque el disipador del SSR1 o el calefactor.

---

## 8. Primer encendido por etapas

Enciende **por etapas**. Si algo falla en una etapa, desenchufa y corrígelo antes de seguir.

### Etapa 0 — Con una carga de prueba en vez del PTC

Para las primeras pruebas, **no conectes el calefactor real**. En su lugar, entre `RL1 NA` y `SSR1 2 OUT+`, pon una **lámpara de 12 V / 21 W** (de auto) o una resistencia de potencia. Es una carga de ensayo, no parte del equipo. Así puedes ver el calentamiento (la lámpara se enciende) sin riesgo.

### Etapa 1 — Sólo la fuente y los reguladores

Con **las placas desconectadas de los reguladores** (`OUT+` de REG_A y REG_B sin cable):

1. ☐ Enchufa la fuente. **No toques cables mientras esté enchufada.**
2. ☐ Mide **12 V** en TB12 respecto a TBG.
3. ☐ Mide **5,0–5,1 V** en la salida de REG_A y de REG_B.
4. ☐ **FAN_B** debe girar. **FAN_P** debe girar (con sus relés sin activar, está encendido).
5. ☐ Ningún componente se calienta ni huele raro.

Si todo es correcto, desenchufa la fuente.

### Etapa 2 — Placas conectadas, aún sin calefactor

1. ☐ Comprueba que los cables de `OUT+` de REG_A (a CONTROL `VIN` y a la pantalla) y de REG_B (a SIS `VIN`) ya están conectados, y que **no hay ningún cable USB** conectado a las placas.
2. ☐ Enchufa la fuente.
3. ☐ La pantalla HMI enciende. En el primer arranque pide calibrar el táctil (toca las esquinas).
4. ☐ **Arriba a la derecha de la pantalla hay un punto**:
   - 🟢 **verde** = la pantalla recibe datos de CONTROL (enlace correcto);
   - 🔴 **rojo** = no hay comunicación con CONTROL. Revisa el enlace ([paso 5B](#5b--enlace-de-datos-conector-i2c-de-4-pines-de-la-pantalla)).
5. ☐ En la pantalla de inicio verás **"Puerta cerrada"** o **"Puerta abierta"**. Abre y cierra la puerta (SW1) y comprueba que **cambia**. Si no cambia o sale "Revisa la puerta", repasa el [paso 4](#paso-4--interruptor-de-la-puerta-sw1).
6. ☐ **En reposo ningún relé debe hacer clic ni activarse** (RL1, RL2, RM1) y la lámpara de prueba debe estar **apagada**. FAN_P y FAN_B giran; FAN_C está parado.

### Etapa 3 — Ciclo de prueba con la carga ficticia

Con la **lámpara** (no el PTC) y la puerta **cerrada**:

1. ☐ En la pantalla: EMPEZAR → elige calzado, intensidad y duración → INICIAR. (La duración más corta, 5 min, es suficiente.)
2. ☐ La pantalla pasa a "Calentando": la lámpara se enciende y se apaga a ratos (el SSR1 regula), FAN_C gira.
3. ☐ **Abre la puerta durante el ciclo**: la lámpara debe **apagarse de inmediato** y la pantalla mostrar la pausa.
4. ☐ Cierra la puerta y reanuda, o cancela. Comprueba que al final el ciclo pasa a "Enfriando" y luego a "Completo".
5. ☐ Desconecta el cable de la pantalla a CONTROL: el punto debe pasar a **rojo** y, al pulsar INICIAR, debe salir un aviso de error.

Si todo lo anterior funciona, la electrónica está correcta.

### Etapa 4 — Con el calefactor real

> ⚠️ Sólo después de que las etapas 1 a 3 salgan bien. Con **supervisión constante**, el calefactor sobre una superficie no inflamable, sin calzado dentro y con un extintor cerca.

1. ☐ Desenchufa la fuente. Quita la lámpara y conecta el **PTC** como en el [paso 9](#paso-9--rama-del-calefactor-ptc).
2. ☐ Revisa de nuevo que los terminales de 12 AWG estén bien apretados.
3. ☐ Haz un ciclo corto **vigilando**: toca (con cuidado) los terminales y cables gruesos tras unos minutos. **Si algún terminal está caliente (no tibio), desenchufa y corrige el apriete.**
4. ☐ Comprueba que el termostato TH1 está montado en el aire caliente, sin tocar las aletas.

---

## 9. Si algo no funciona

| Síntoma | Causa probable | Qué hacer |
|---|---|---|
| La pantalla no enciende | Sin 5 V en el USB-C o polaridad invertida | Mide REG_A (5,0 V) y revisa `5V`/`GND` de la pantalla |
| El punto de la pantalla está **rojo** | No llegan datos de CONTROL | Revisa [paso 5B](#5b--enlace-de-datos-conector-i2c-de-4-pines-de-la-pantalla): TX↔RX cruzados, GND común, CONTROL encendido con su firmware |
| Pantalla verde pero "Revisa la puerta" | Cables `NA`/`NC` de la puerta cortados o mal conectados | Repasa el [paso 4](#paso-4--interruptor-de-la-puerta-sw1) |
| "Puerta abierta" siempre (o "cerrada" siempre) | `NA` y `NC` intercambiados, o `COM` sin tierra | Revisa SW1 |
| Falla de sensor de temperatura | Termopar con polaridad invertida o mal conectado; `SCK`/`SO`/`CS` mal | Repasa el [paso 3B](#3b--termopares-temperatura-del-aire) |
| Humedad u olor sin datos ("--") | SHT31/SGP40 sin alimentar o `SDA`/`SCL` mal | Repasa el [paso 3C](#3c--sht31-y-sgp40-humedad-y-olor) |
| La lámpara/PTC nunca enciende | SIS no da permiso (RL1 abierto) o SSR1 sin señal | Escucha si RL1 hace clic; revisa `D25` de SIS y de CONTROL |
| La lámpara está encendida sin ciclo | SSR1 en cortocircuito o `D25` mal conectado | **Desenchufa.** Revisa SSR1; si está en corto, sustitúyelo |
| "Falla interna de comunicación" | Enlace entre CONTROL y SIS caído | Revisa [paso 6](#paso-6--placa-sis): TX↔RX cruzados y GND común |
| FAN_P no gira al encender | Relé NC mal conectado o RL2/RL3 activos | Revisa el [paso 8B](#8b--fan_p-ventilador-del-calefactor): COM a 12 V, NC a FAN_P |
| Un módulo se calienta o huele | Cortocircuito o 12 V en un pin de señal | **Desenchufa** y busca el cable |
| La placa se reinicia sola | 5 V insuficientes o ajustados a otro valor | Mide el regulador con carga; cables finos o flojos |

---

## 10. Puntos que aún hay que verificar

Estos puntos están marcados `[VERIFICAR]` en el [plan maestro](../PLAN-MAESTRO.md): dependen de los módulos concretos que se compren y **no se pueden asegurar desde el papel**. Compruébalos con el multímetro cuando tengas las piezas.

| Pendiente | Qué comprobar |
|---|---|
| **Orden de los 4 pines del conector I2C de la pantalla** | Con el multímetro, identifica cuál es `IO25`, `IO32`, `GND` y `3V3` (dos pines concretos van a esos GPIO). No conectes `3V3` |
| **USB y regulador a la vez** | Si la placa tiene un diodo que aísle el USB del pin `VIN`. Si no, **nunca** USB y regulador a la vez |
| **Módulos de relé con 3,3 V** | Que RL1, RL2 y RM1 se activen con 3,3 V y no se activen con la entrada al aire (ver [§5.4](#-54-comprueba-cada-módulo-de-relé-y-cada-ssr-en-frío)) |
| **SSR con 3,3 V** | Que SSR1 y SSR2 se activen con 3,3 V en su entrada |
| **Configuración del SSR** | Que el SSR de tu modelo funcione "en el lado de tierra" como en el diagrama (carga entre +12 V y `OUT+`) |
| **Corriente de los contactos de RL1 y TH1 en continua** | Muchos datos se dan en CA; deben aguantar ≈ 8,3 A en DC |
| **Corriente de arranque del PTC** | Que la fuente de 20 A la soporte (se mide en pruebas) |
| **Tensión de la bobina de RM1** | 12 V o 5 V según el módulo (el diagrama supone 12 V) |

> **Sobre el termostato TH1**: el diagrama lo conecta **en serie con la línea de 12 V del calefactor** (opción "T1" del [plan maestro](../PLAN-MAESTRO.md), decisión A-11, la recomendada). Si el proyecto cambia a la opción T2, la señal de `RL1.IN` pasa por TH1 y esta guía debe actualizarse.
>
> **Sobre los termopares**: el equipo tiene **dos** termopares (TC1 y TC2, en CONTROL). El SIS **no** tiene termopar propio. (Algunas partes antiguas del plan maestro mencionan un TC3: está desactualizado.)

---

## Resumen: tabla de cables de señal entre placas

Para tener todo a mano:

| Señal | Desde | Hacia |
|---|---|---|
| CONTROL → SIS (datos) | CONTROL `D17` (TX2) | SIS `D16` (RX2) |
| SIS → CONTROL (datos) | SIS `D17` (TX2) | CONTROL `D16` (RX2) |
| CONTROL → pantalla (datos) | CONTROL `D4` | Pantalla `IO32` |
| Pantalla → CONTROL (datos) | Pantalla `IO25` | CONTROL `D35` |
| Puerta `NA` | SW1 `NA` | CONTROL `D32` y SIS `D32` |
| Puerta `NC` | SW1 `NC` | CONTROL `D33` y SIS `D33` |
| Tierra común | GND de CONTROL, SIS y pantalla | TBG |

Y los pines de las salidas:

| Placa | Pin | Va a | Significa |
|---|---|---|---|
| CONTROL | `D25` | SSR1 `IN+` | Calentar |
| CONTROL | `D26` | SSR2 `IN+` | Ventilador de circulación |
| CONTROL | `D27` | RL2 `IN` | Pedir apagar FAN_P |
| SIS | `D25` | RL1 `IN` | Permiso para calentar |
| SIS | `D26` | RM1 `IN1` (RL3) | Permitir apagar FAN_P |
| SIS | `D27` | RM1 `IN2` (RL4) | Forzar FAN_C |

Para más detalle técnico: [pinout/](../pinout/README.md), [comunicacion.md](../comunicacion.md) y el [plan maestro](../PLAN-MAESTRO.md#5-electrónica).
