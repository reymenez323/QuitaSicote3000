# ADR-0010 — Permiso del PTC con un módulo de relé en serie

**Estado:** Aceptada (decisión del usuario). Sustituye a la ADR-0003, retirada.

## Contexto
El SIS necesita cortar el PTC por su cuenta, incluso si el SSR1 del control falla en cortocircuito (su modo de falla típico).

## Decisión
- Un **módulo de relé de 12 V (RL1)** en serie con el PTC: TH1 → RL1 (contacto NA) → PTC → SSR1 → GND.
- RL1 lo maneja **sólo el SIS** (ESP32 Dev Kit), con **disparo por nivel alto** compatible con 3,3 V: energizado = permitido. Con el SIS arrancando o apagado, su entrada queda al aire y RL1 debe quedar abierto [VERIFICAR en F3].
- RL1 se cierra al empezar a calentar y se abre al terminar; no conmuta en cada ventana del SSR1 (desgaste de contactos).

## Requisitos del módulo
- Contacto apto para la corriente del PTC **en DC** (≈ 8,3 A en régimen, más en el arranque): módulo de **30 A**. Los módulos comunes de 10 A no sirven [VERIFICAR la especificación DC del relé].

## Consecuencias
- (+) El SIS corta el PTC aunque el SSR1 esté en cortocircuito.
- (−) Sin lectura de 12 V (ADR-0013), un contacto soldado de RL1 no se detecta. Se revisa en la validación periódica.
