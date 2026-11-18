Driver de la placa auxiliar Raspberry Pi — DSO

Proyecto en desarrollo. Módulo base con carga y descarga; todavía no
implementa los dispositivos ni reserva GPIO.

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
