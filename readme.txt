Driver de la placa auxiliar Raspberry Pi — DSO

Proyecto en desarrollo. Implementa /dev/leds y /dev/speaker;
buttons pendiente.

Objetivo: un módulo de kernel con /dev/leds, /dev/speaker y /dev/buttons.
Material del profesor y programas de prueba: carpeta superior, sin modificar.

Pasos acordados: docs/plan.md.

Ficheros:
  driver_rpi.c  Código del módulo.
  Makefile      Compilación, carga, descarga y limpieza.

Uso en la Raspberry Pi, con las cabeceras de su kernel instaladas:
  make
  make install
  sudo dmesg | tail
  make uninstall
  sudo dmesg | tail
  make clean

Compilar desde una ruta sin espacios (por ejemplo ~/driver-rpi), como
requiere Kbuild. El .ko debe compilarse para el kernel donde se cargará.
Para elegir otra carpeta de cabeceras: make KERNEL_SRC=/ruta/al/kernel

No cargar este módulo en el PC para comprobar el hardware de la Raspberry.

LEDs:
  Bits 0..5: GPIO BCM 9, 10, 11, 17, 22 y 27, respectivamente.
  Numeración Linux: base 512, según los ejemplos del profesor.
  Antes de cargar, confirmar la base en sudo cat /sys/kernel/debug/gpio.
  Si difiere, ajustar GPIO_BASE en driver_rpi.c y recompilar.
  Los LEDs empiezan apagados y se apagan al descargar.

  Lectura: un byte binario con el estado; después devuelve EOF.
  Escritura: consume un byte binario por llamada, no texto ASCII.
  Bits 7..6:
    00: asignar los seis bits.
    01: encender los LEDs marcados.
    10: apagar los LEDs marcados.
    11: invertir los LEDs marcados.

  Ejemplos en la Raspberry:
    sudo sh -c 'printf "\001" > /dev/leds'  # encender solo LED 1
    sudo sh -c 'printf "\102" > /dev/leds'  # añadir LED 2
    sudo sh -c 'printf "\201" > /dev/leds'  # apagar LED 1
    sudo sh -c 'printf "\302" > /dev/leds'  # invertir LED 2
    sudo od -An -t x1 /dev/leds

Altavoz:
  /dev/speaker es de solo escritura, conectado al GPIO BCM 4
  (GPIO Linux 516 con la base actual).
  Cada escritura consume un byte: ASCII '0' apaga, cualquier otro activa.
  Se pueden hacer escrituras sucesivas con el mismo descriptor.
  La salida empieza a cero y se apaga al descargar el módulo.
  El driver no genera tonos: la aplicación debe alternar '0' y '1'.

  Para usar los programas del profesor desde la carpeta superior:
    gcc prueba_speaker2.c -o /tmp/prueba_speaker2 -pthread
    sudo /tmp/prueba_speaker2
    gcc sonidoTetris.c -o /tmp/sonidoTetris -pthread
    sudo /tmp/sonidoTetris

  Si una aplicación deja la salida activa, apagarla con:
    sudo sh -c 'printf "0" > /dev/speaker'
