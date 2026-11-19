Driver de la placa auxiliar Raspberry Pi — DSO

Proyecto en desarrollo. Implementa /dev/leds, /dev/speaker y
/dev/buttons con apertura exclusiva. Pendiente validación en la Raspberry.

Módulo de kernel para Linux 6.6 del laboratorio.
Los programas de prueba del profesor se proporcionan por separado.

Ficheros:
  driver_rpi.c  Código del módulo.
  Makefile      Compilación, carga, descarga y limpieza.
  readme.txt    Descripción y uso.

leds y speaker se registran como dispositivos misc. buttons usa un cdev
independiente para que su open bloqueante no retenga el mutex global de misc.

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

  Para usar los programas del profesor, desde la carpeta con sus fuentes:
    gcc prueba_speaker2.c -o /tmp/prueba_speaker2 -pthread
    sudo /tmp/prueba_speaker2
    gcc sonidoTetris.c -o /tmp/sonidoTetris -pthread
    sudo /tmp/sonidoTetris

  Si una aplicación deja la salida activa, apagarla con:
    sudo sh -c 'printf "0" > /dev/speaker'

Botones:
  /dev/buttons es de solo lectura.
  Botón 1: GPIO BCM 2 (Linux 514); botón 2: BCM 3 (Linux 515).
  Cada lectura devuelve un carácter ASCII '1' o '2', en orden de llegada.
  Si no hay pulsaciones, espera; CTRL+C puede interrumpir la espera.
  Con O_NONBLOCK, una lectura sin datos devuelve EAGAIN.
  Una lectura de cero bytes no espera ni consume pulsaciones.

  Interrupciones por flanco de bajada, con hilo para guardar la pulsación
  en una FIFO de 64 caracteres y despertar al lector. Si se llena,
  se descartan las nuevas pulsaciones hasta que haya espacio.
  Antirrebote independiente por botón: 50 ms por defecto.
  Para cambiarlo al cargar:
    sudo insmod driver_rpi.ko debounce_ms=80

  Para usar la prueba del profesor, compilar desde la carpeta con su fuente:
    gcc lee_buttons.c -o /tmp/lee_buttons
    sudo /tmp/lee_buttons
  Leerá cinco pulsaciones y cerrará el dispositivo.
  También se puede usar: sudo cat /dev/buttons

  Apertura exclusiva: solo se permite una apertura simultánea.
  Si está ocupado, open espera hasta que se cierre; CTRL+C puede
  interrumpir esa espera. Con O_NONBLOCK, open devuelve EBUSY.
  El acceso se libera al cerrar el último descriptor de esa apertura,
  también si el proceso termina. Cambiar O_NONBLOCK no impide liberarlo.

  Para comprobarlo con la prueba del profesor:
    sudo /tmp/lee_buttons dos
  Se crean dos procesos. El segundo espera a que el primero lea cinco
  pulsaciones y cierre; después puede leer sus cinco pulsaciones.
