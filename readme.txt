Driver de la placa auxiliar Raspberry Pi - Manuel Laguna Martínez y Salvador Bustamante Garnica

Módulo de kernel que controla seis LEDs, un altavoz y dos botones.

Ficheros:
  driver_rpi.c  Código del controlador.
  Makefile      Compilación, carga, descarga y limpieza.
  readme.txt    Descripción del proyecto.

/dev/leds
  Lee un byte con el estado de los seis LEDs en los bits 0..5.
  Al escribir, los bits 7..6 indican la operación sobre los bits 0..5:
    00: asignar el estado.
    01: encender los LEDs marcados.
    10: apagar los LEDs marcados.
    11: invertir los LEDs marcados.

/dev/speaker
  Solo escritura. El carácter '0' desactiva la salida y cualquier otro
  byte la activa. La aplicación genera el sonido alternando la salida.

/dev/buttons
  Solo lectura. Devuelve '1' o '2' por cada pulsación.
  Usa interrupciones con hilo, una FIFO de 64 caracteres y antirrebote
  configurable mediante debounce_ms (50 ms por defecto).
  La lectura espera si no hay pulsaciones; con O_NONBLOCK devuelve EAGAIN.
  La apertura es exclusiva: espera si está ocupado o devuelve EBUSY
  con O_NONBLOCK. Las esperas pueden interrumpirse mediante señales.
