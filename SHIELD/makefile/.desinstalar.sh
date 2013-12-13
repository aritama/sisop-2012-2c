#!/bin/bash

if [ "$(whoami)" != "root" ]
then
    echo -e "\nSolo usuario con privilegios root debe desinstalar\n"
    exit 1
fi

if [ ! -h /usr/bin/shield.sh ]; then echo -e "\nShield no se encuentra instalado\n" ; exit 1; fi;

INSTALL_PATH=$(readlink -f /usr/bin/shield.sh)
INSTALL_PATH=${INSTALL_PATH%/*}
INSTALL_PATH=${INSTALL_PATH%/*}

if [ -f ${INSTALL_PATH}/.configured_users ]
then
    if [ -n "$(cat ${INSTALL_PATH}/.configured_users | grep :)" ]
    then 
	echo -e "\nTodavía hay usuarios configurados a Shield"
	echo "Usuarios configurados: "
	CONFIGURADOS="$(cat ${INSTALL_PATH}/nucleo/ShieldUsers | tr '\n' ' ')"
	for Usuario in ${CONFIGURADOS}
	do
	    echo "    ${Usuario}"
	done
	echo -e "\n"
	exit 1
    fi
fi

rm -f -r ${INSTALL_PATH}
INSTALL_PATH=${INSTALL_PATH%/*}

# Las pasaríamos a ver uno por uno a ver si borramos todo, si tiene cosas por ahi ya estaban de antes, entonces no
function borrarDirectorio
{
	while [ -n "${INSTALL_PATH}" ]
	do
		if [ -z "$(ls -A ${INSTALL_PATH})" ]
		then
			rm -f -r ${INSTALL_PATH}
			INSTALL_PATH=${INSTALL_PATH%/*}
		else
			break
		fi
	done
}

borrarDirectorio
rm -f /usr/bin/shield.sh

sed -i "s|/usr/bin/shield.sh||g" /etc/shells
sed -i '/^$/d' /etc/shells

echo -e "\nShield desinstalado\n"

exit 0
