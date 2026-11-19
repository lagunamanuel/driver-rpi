#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"

if [[ ! -r /proc/device-tree/model ]] ||
   ! grep -aq 'Raspberry Pi' /proc/device-tree/model; then
	echo "Ejecuta este script en la Raspberry Pi." >&2
	exit 1
fi
if ! grep -aEq 'brcm,bcm(2835|2836|2837|2711)' /proc/device-tree/compatible; then
	echo "Este driver solo contempla las Raspberry con GPIO BCM283x/BCM2711." >&2
	exit 1
fi
if [[ "$(pwd -P)" =~ [[:space:]] ]]; then
	echo "Kbuild requiere una ruta sin espacios; clona en ~/driver-rpi." >&2
	exit 1
fi

for tool in make gcc insmod; do
	if ! command -v "$tool" >/dev/null; then
		echo "Falta $tool. Instala las herramientas de compilación y kmod." >&2
		exit 1
	fi
done

kernel=$(uname -r)
if [[ ! -f "/lib/modules/$kernel/build/Makefile" ]]; then
	echo "Faltan las cabeceras del kernel $kernel." >&2
	echo "Instala las cabeceras que coincidan con uname -r y vuelve a ejecutar." >&2
	exit 1
fi
if [[ -d /sys/module/driver_rpi ]]; then
	echo "El módulo ya está cargado. Cierra sus programas y usa sudo rmmod driver_rpi." >&2
	exit 1
fi

privilege=()
if (( EUID != 0 )); then
	command -v sudo >/dev/null || { echo "Necesitas sudo." >&2; exit 1; }
	privilege=(sudo)
fi

make clean
make
echo "Antes de cargar: confirma el cableado de la placa auxiliar y GPIO_BASE."
echo "La base debe coincidir con sudo cat /sys/kernel/debug/gpio."
read -r -p "¿Lo has comprobado y quieres cargar el módulo? [s/N] " answer
case "$answer" in
	s|S) "${privilege[@]}" insmod ./driver_rpi.ko "$@" ;;
	*) echo "Módulo compilado, sin cargar."; exit 0 ;;
esac

echo "Módulo cargado: /dev/leds, /dev/speaker y /dev/buttons."
echo "Para descargarlo: sudo rmmod driver_rpi"
echo "No se ha configurado la carga al arrancar."
