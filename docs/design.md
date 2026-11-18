# Diseño acordado

Implementar el enunciado del laboratorio con código C mínimo, pocos
comentarios y sin añadir tests. Avanzar por entregas con commits atómicos,
esperando «sigue» entre entregas.

## Enfoque

Un módulo, `driver_rpi`, en un único fichero C y tres dispositivos misc.
Usar la API GPIO de los ejemplos del profesor; confirmar en la Raspberry
la base GPIO (los ejemplos usan 512).

Para los botones se eligen interrupciones con hilo frente a tasklets o
workqueues: cumplen el trabajo diferido sin añadir otra infraestructura.
FIFO del kernel protegida frente a concurrencia, cola de espera para
lecturas y semáforo para apertura exclusiva.

## Comportamiento

- leds: lectura de un byte con bits 7–6 a cero; escritura binaria con
  operaciones asignar, encender, apagar e invertir según bits 7–6.
- speaker: solo escritura; ASCII '0' desactiva y cualquier otro byte activa.
  Debe aceptar escrituras sucesivas desde el mismo descriptor.
- buttons: solo lectura; caracteres '1' y '2', interrupciones por flanco
  de bajada y antirrebote independiente por botón. Se propone un parámetro
  `debounce_ms` de 50 ms por defecto.
- Lectura de botones vacía: espera interrumpible; con O_NONBLOCK, -EAGAIN.
- Apertura exclusiva: con O_NONBLOCK, -EBUSY si está ocupado; sin él,
  espera interrumpible hasta el cierre.
- Liberar los recursos adquiridos tanto al descargar como ante errores.

## Alcance

Repositorio privado `driver-rpi`, separado del material original.
Sin dependencias adicionales, generación de sonidos en kernel ni tests
nuevos. Las comprobaciones de hardware se harán con la Raspberry y los
programas del profesor; no se afirmará funcionamiento físico sin probarlo.
