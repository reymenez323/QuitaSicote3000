# ADR-0007 — Los perfiles de tratamiento viven en el control; el SIS tiene un límite único

**Estado:** Propuesta

## Decisión
- La traducción *(tipo de calzado, intensidad, duración)* → *(temperatura objetivo, límites, duración efectiva)* se hace **en la Mega**. El HMI sólo envía identificadores.
- El SIS mantiene **un único umbral de sobretemperatura fijo** (constante de compilación), independiente del perfil.

## Razón
Si el SIS aceptara el perfil por la red, un mensaje corrupto podría relajar su umbral. Un único umbral es más simple y más fácil de validar.

## Costo
El umbral único debe cubrir la intensidad más alta de todos los perfiles, así que para calzado delicado (cuero a 35 °C) el SIS protege con menos margen que el control. Las capas 1 (software de la Mega) y 2 (SIS) quedan separadas por esa diferencia.

## Alternativa (revisar después de ensayos)
Una tabla fija de 2–3 niveles de umbral dentro del firmware del SIS, seleccionada por un código recibido; con valor por defecto, ante dato inválido o ausente, igual al **nivel más restrictivo**. Sólo vale la pena si los ensayos muestran un daño real al calzado entre el nivel bajo y el umbral único.
