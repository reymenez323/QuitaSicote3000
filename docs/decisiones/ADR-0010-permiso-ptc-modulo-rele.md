# ADR-0010 — Permiso del PTC con un módulo de relé en serie

**Estado:** Aceptada (decisión del usuario). Sustituye a la ADR-0003, retirada.

## Contexto
El SIS necesita cortar el PTC por su cuenta, incluso si el SSR1 de la Mega falla en cortocircuito (su modo de falla típico).

## Decisión
- Un **módulo de relé de 12 V (RL1)** en serie con el PTC: TH1 → RL1 (contacto NA) → PTC → SSR1 → GND.
- RL1 lo maneja **sólo el Nano** (D4), con **disparo por nivel alto**: energizado = permitido. Con el Nano reiniciando o apagado, RL1 queda abierto.
- RL1 se cierra al empezar a calentar y se abre al terminar; no conmuta en cada ventana del SSR1 (desgaste de contactos).

## Requisitos del módulo
- Contacto apto para la corriente del PTC **en DC** (≈ 8,3 A en régimen, más en el arranque): módulo de **30 A**. Los módulos comunes de 10 A no sirven [VERIFICAR la especificación DC del relé].
- Debe quedar **desactivado con la entrada al aire** [VERIFICAR en F3].

## Consecuencias
- (+) El SIS corta el PTC aunque el SSR1 esté en cortocircuito.
- (−) Sin componentes auxiliares (ADR-0011) no hay realimentación: un contacto de RL1 soldado no se detecta. Se revisa en la validación periódica.
