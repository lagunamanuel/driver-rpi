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
- [x] 1. Crear `driver_rpi.c` con init/exit y metadatos; `Makefile` con
  compilación, instalación, descarga y limpieza; `.gitignore` de Kbuild.
  Actualizar `readme.txt`, comprobar compilación si hay cabeceras adecuadas
  y registrar explícitamente cualquier limitación. Commit de la base.
- [x] 2. Implementar /dev/leds: seis GPIO según material, read de un byte,
  write con cuatro operaciones y protección de actualizaciones.
  Documentar el uso y comprobar compilación. Commit de LEDs.
- [x] 3. Implementar /dev/speaker: GPIO de salida, escritura de un byte
  comparado con ASCII '0', sin limitar escrituras sucesivas.
  Documentar y comprobar compilación. Commit del altavoz.
- [x] 4. Implementar /dev/buttons con FIFO, sincronización, IRQ con hilo,
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

## Registro

Entrega 1: módulo base, Makefile, .gitignore y documentación.
Comprobado `make W=1`, `modinfo driver_rpi.ko` y `make clean` contra
7.3.3-300.fc45.x86_64, en copia temporal sin espacios. Compilación correcta;
Kbuild omite BTF por no disponer de vmlinux. No se ha cargado el módulo.
Pendiente compilar y comprobar carga/descarga en la Raspberry.
Se mantiene el trabajo en el repositorio creado y se pausa por entregas,
sin tests nuevos, según lo solicitado por el usuario.

Entrega 2: /dev/leds con estado binario de seis bits, cuatro operaciones
de escritura y mutex para lecturas y actualizaciones del estado.
Pines y orden tomados de 1_leds_device_driver/3_class_dev.c.
Reserva de GPIO antes de publicar el dispositivo, liberación parcial ante
errores y apagado al descargar. Sin tests nuevos.
Comprobados `make W=1`, `modinfo` y `make clean` contra el kernel del PC
7.3.3-300.fc45.x86_64; BTF omitido por falta de vmlinux.
Se eliminó la referencia a no_llseek, retirada en kernels modernos;
se deja llseek sin definir como indica la documentación del kernel.
Pendiente validar en la Raspberry, incluida la base GPIO 512.

Entrega 3: /dev/speaker, solo escritura, GPIO BCM 4 (Linux 516),
según 2_shell_io/sysfs_interface.sh. ASCII '0' desactiva y cualquier
otro byte activa, sin limitar escrituras sucesivas por descriptor.
Salida inicial y final a cero; liberación de recursos ante errores.
Comprobados `make W=1`, `modinfo` y `make clean` contra el kernel del PC
7.3.3-300.fc45.x86_64; BTF omitido por falta de vmlinux.
Compilados prueba_speaker2.c y sonidoTetris.c con -pthread, sin modificarlos
ni ejecutarlos. No se han añadido tests ni cargado el módulo en el PC.
Pendiente comprobar los sonidos en la Raspberry.

Entrega 4: /dev/buttons devuelve un carácter por lectura ('1' o '2').
GPIO BCM 2 y 3, según 2_shell_io/sysfs_interface.sh.
IRQ por flanco de bajada con filtrado temporal independiente por botón;
el hilo de IRQ guarda las pulsaciones en una kfifo de 64 elementos y
despierta la cola de espera. debounce_ms=50, configurable al cargar.
Spinlock para la FIFO y mutex para serializar lectores, incluso si
comparten descriptor; copy_to_user se ejecuta fuera del spinlock y no
consume la pulsación si falla. Lecturas vacías bloqueantes e interrumpibles;
O_NONBLOCK devuelve EAGAIN. FIFO llena: descartar nuevas pulsaciones.
Liberación de IRQ antes del GPIO, también ante inicialización parcial.
Comprobados `make W=1`, `modinfo` (incluido debounce_ms) y `make clean`
contra 7.3.3-300.fc45.x86_64, más compilación de lee_buttons.c sin cambios.
BTF omitido por falta de vmlinux. Sin tests nuevos ni carga del módulo.
Pendiente prueba física y apertura exclusiva (entrega 5).
