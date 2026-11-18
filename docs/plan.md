# Driver Raspberry Pi — Implementation Plan

> Ejecución: usar executing-plans, una entrega por turno.

**Objetivo:** implementar el driver solicitado sin funcionalidad adicional.
**Arquitectura:** un módulo C, tres misc devices y botones mediante IRQ con hilo.
**Tecnologías:** C, Linux kernel, Kbuild y GitHub.
**Diseño:** [design.md](design.md).

## Restricciones

- Código mínimo, comentarios necesarios y sin añadir tests.
- No modificar los materiales del profesor.
- Commits atómicos y pausa hasta que el usuario diga «sigue».
- Compilar para el kernel de la Raspberry, no confundirlo con el del PC.

## Puntos de revisión

- Numeración GPIO y cableado reales antes de usar hardware.
- Lecturas/escrituras de tamaño cero y punteros de usuario inválidos.
- Escrituras sucesivas al altavoz sin EOF artificial.
- Concurrencia, señales y apertura exclusiva de botones.
- Liberación parcial de recursos ante errores de inicialización.

## Entregas

- [x] 0. Crear repositorio privado con descripción y pasos acordados.
- [ ] 1. Crear `driver_rpi.c` con init/exit y metadatos; `Makefile` con
  compilación, instalación, descarga y limpieza; `.gitignore` de Kbuild.
  Actualizar `readme.txt`, comprobar compilación si hay cabeceras adecuadas
  y registrar explícitamente cualquier limitación. Commit de la base.
- [ ] 2. Implementar /dev/leds: seis GPIO según material, read de un byte,
  write con cuatro operaciones y protección de actualizaciones.
  Documentar el uso y comprobar compilación. Commit de LEDs.
- [ ] 3. Implementar /dev/speaker: GPIO de salida, escritura de un byte
  comparado con ASCII '0', sin limitar escrituras sucesivas.
  Documentar y comprobar compilación. Commit del altavoz.
- [ ] 4. Implementar /dev/buttons con FIFO, sincronización, IRQ con hilo,
  antirrebote configurable y lectura bloqueante interrumpible.
  Comprobar compilación y documentar el uso de `lee_buttons.c`.
  Commit de lectura de botones.
- [ ] 5. Añadir apertura exclusiva con semáforo, espera interrumpible,
  -EBUSY con O_NONBLOCK y liberación en release. Comprobar compilación.
  Commit de exclusividad.
- [ ] 6. Revisar contra el enunciado, completar `readme.txt` y comprobar
  con los programas del profesor cuando esté disponible la Raspberry.
  Preparar instrucciones para entregar fuentes, Makefile y readme en ZIP.

No añadir programas de prueba. Cada entrega requiere comprobar el diff
y subir su commit; las pruebas físicas pendientes se indicarán como tales.
