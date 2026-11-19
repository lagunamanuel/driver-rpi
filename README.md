# Driver Raspberry Pi

Controlador de la placa auxiliar del laboratorio: LEDs, altavoz y botones.
El uso de los dispositivos está en [readme.txt](readme.txt).

## Instalar para probar

En la Raspberry, desde una carpeta sin espacios:

```sh
git clone https://github.com/lagunamanuel/driver-rpi.git
cd driver-rpi
bash instalar.sh
```

El script comprueba la placa, las herramientas y las cabeceras del kernel
en ejecución, compila y pide confirmación antes de cargar con `insmod`.
Requiere `make`, `gcc`, `kmod` y cabeceras que coincidan con `uname -r`.
No instala paquetes, actualiza el sistema ni modifica la configuración.

Confirmar el cableado y la base GPIO antes de aceptar la carga. Se usa
base 512 por defecto, como en los ejemplos del laboratorio; si difiere,
ajustar `GPIO_BASE` en `driver_rpi.c`. No usar en Raspberry Pi 5.

Para elegir otro intervalo de antirrebote:

```sh
bash instalar.sh debounce_ms=80
```

Para ver los mensajes o descargar:

```sh
sudo dmesg | tail
sudo rmmod driver_rpi
```

Antes de descargar, cerrar los programas que estén usando los dispositivos.
Tras reiniciar, hay que volver a ejecutar el script: no hay carga automática.

## Comprobar en la placa

```sh
sudo sh -c 'printf "\001" > /dev/leds'
sudo od -An -t x1 /dev/leds
sudo cat /dev/buttons
```

El primer LED debe encenderse y la lectura debe mostrar `01`.
Al pulsar los botones deben aparecer `1` y `2`; salir con CTRL+C.
Para probar los sonidos y dos lectores, copiar aparte los programas del
profesor y seguir `readme.txt`: esos programas no están en el repositorio.

## Entrega

El ZIP contiene solo `driver_rpi.c`, `Makefile` y `readme.txt`.
El instalador y este documento son auxiliares y quedan fuera de la entrega.

La compilación se ha comprobado en el PC; la validación del hardware y del
comportamiento bloqueante queda pendiente en la Raspberry.
