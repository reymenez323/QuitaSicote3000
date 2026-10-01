# ADR-0009 — Ventilador del PTC conmutado por módulo de relé de 12 V

**Estado:** Propuesta

## Contexto
El usuario quiere que el ventilador del PTC (12 V) se encienda y apague mediante un **módulo de relé de 12 V**. Antes (alimentación directa) giraba siempre que hubiera 12 V. Ahora **puede quedar apagado con el PTC energizado** por un error de software, una falla del relé o un cable suelto, lo que es una de las causas principales de sobrecalentamiento (P-02, P-15).

## Decisión propuesta

1. **Contacto NC del relé**: el ventilador va conectado a COM + NC. Relé sin energizar = ventilador **encendido**. Si falla la señal de la Mega o la bobina no se energiza por cualquier causa, el ventilador gira por defecto.
2. La Mega energiza el relé (ventilador **apagado**) sólo cuando se cumplen las dos: PTC apagado y TC3 por debajo de una temperatura fría T_FRIO (p. ej. 35 °C [VERIFICAR]) durante un tiempo.
3. **El SIS tiene un "permiso de apagado"** en serie con la señal de la Mega hacia el módulo (compuerta AND, ver "Configuración elegida"). Si el SIS no está vivo o no concede el permiso, la señal no llega y el ventilador sigue encendido. El SIS no concede el permiso mientras el permiso del PTC esté activo, TC3 > T_FRIO o haya un disparo.
4. **Sensado**: el SIS lee con un divisor resistivo la alimentación del ventilador del PTC (nodo tras el relé). Con el PTC energizado y ese nodo sin tensión, disparo (parte de SIF-03). Es un par de resistencias, no un dispositivo nuevo.
5. En AUTOTEST la Mega conmuta el relé y el SIS comprueba que el nodo cambia.

## Configuración elegida (el módulo admite ambas)

- **Contacto**: NC (COM + NC al ventilador). Razón: reposo = ventilador encendido.
- **Disparo por nivel ALTO** (jumper H/L del módulo en "H"): energizado = ventilador **apagado**. Razón: cuando una entrada flota o una MCU se reinicia, el estado natural es nivel bajo, y eso deja el relé sin energizar, es decir, el ventilador encendido. Con disparo por nivel bajo, una entrada flotante o un pin en alta impedancia podría energizar el relé.
- **Compuerta AND** (p. ej. un 74HC08) entre la salida de la Mega y la entrada del módulo, con la segunda entrada gobernada por el SIS ("permiso de apagado"). **Pull-down en las dos entradas de la compuerta y en la entrada del módulo.** Si falta la Mega, el SIS o la alimentación de la compuerta, la salida es baja y el ventilador sigue girando.
- Si el módulo tiene separación por optoacoplador (VCC/JD-VCC), alimentar la parte lógica desde los 5 V del control y la bobina desde los 12 V.
- Equivalente para forzar el ventilador de circulación (SSR de entrada activa en alto): **OR** entre la salida de la Mega y la orden del SIS, con pull-downs.

## A verificar del módulo
- Nivel de disparo (activo alto o bajo) y tensión de la entrada (muchos módulos de 12 V usan optoacoplador y admiten 5 V; confirmar con el modelo exacto).
- Que tenga contacto **NC** accesible (casi todos lo tienen: COM/NO/NC) y que sus contactos soporten la corriente del ventilador.
- Los ventiladores son de 12 V y 2 cables (sin tacómetro): **no se puede medir el giro**. El sensado del SIS detecta sólo que hay tensión en el ventilador, no que gire; un ventilador trabado o desconectado a medias se detectaría únicamente por la subida de TC3 (SIF-03). Si resultaran ser de escobillas, añadir un diodo antiparalelo.

## Consecuencias
- (+) El control puede apagar el ventilador tras enfriar (ruido, polvo).
- (+) Fallo de relé, de señal o del SIS ⇒ ventilador encendido.
- (−) Con NC, durante todo el tiempo en reposo la bobina está energizada (≈ 70 mA a 12 V); es aceptable, pero hay que medirlo si se alimenta de baterías (no es el caso).
- (−) Un transistor/compuerta y un divisor más en el esquema.
- (−) El relé de un módulo comercial suele tener contactos mediocres; para un equipo doméstico conviene un relé de marca y revisar su vida útil (pocas conmutaciones al día, no es crítico).

## Alternativa (descartada por el usuario)
Alimentación directa al riel de 12 V, sin ningún elemento de maniobra.
