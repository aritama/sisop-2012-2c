#!/bin/bash

if [ "$(whoami)" != "root" ]
then
    echo -e "\nSolo usuario con privilegios root debe instalar\n"
    exit 1
fi

INSTALL_PATH='/etc/shield'

if [ -h /usr/bin/shield.sh ]; then echo -e "\nShield ya se encuentra instalado\n"; exit 1; fi;

read -e -p "Ingrese el path (absoluto de ser posible) de instalación (enter para default): " INPUT_PATH
if [ -z ${INPUT_PATH##* } ]; then INPUT_PATH=''; fi;


if [ -n "${INPUT_PATH}" ]; then INSTALL_PATH="${INPUT_PATH}"; fi;


INPUT_PATH="$(echo "$INSTALL_PATH" | tr '/' ' ')"
INSTALL_PATH=''
ERROR=0
for WORD in $INPUT_PATH
do
    if [ -z "$INSTALL_PATH" ]; then INSTALL_PATH="/${WORD}"; else INSTALL_PATH="${INSTALL_PATH}/${WORD}"; fi;
    if [ ! -d "$INSTALL_PATH" ] && [ "${WORD}" != "shield" ]
    then 
        mkdir $INSTALL_PATH 
        if [ $? -ne 0 ]
	then 
		echo -e "\nError en la instalación\n"
		INSTALL_PATH=${INSTALL_PATH%/*}
		ERROR=1
		break
	fi
    fi
done
function trataERROR
{
    if [ $ERROR -eq 1 ]
    then
	while [ -n "${INSTALL_PATH}" ]
	do
	    if [ -z "$(ls -A ${INSTALL_PATH})" ]
	    then
		rm -f -r ${INSTALL_PATH}
	    else
		break
	    fi
	    INSTALL_PATH=${INSTALL_PATH%/*}
	done
	exit 1
    fi
}
trataERROR
if [ "${INSTALL_PATH##*/}" != "shield" ]; then INSTALL_PATH="${INSTALL_PATH}/shield"; fi;
if [ -d ${INSTALL_PATH} ]
then
    if [ -n "$(ls -A ${INSTALL_PATH})" ]
    then
	echo -e "\nAbortando instalación, existe el directorio shield dentro del path de instalación y no se encuenta vacía, pruebe eligiendo otro path por seguridad\n"
	ERROR=1
    else
	rmdir ${INSTALL_PATH}
	mkdir ${INSTALL_PATH}
    fi
else
    mkdir ${INSTALL_PATH}
fi
trataERROR

echo -e "\nComenzando instalación en el path: ${INSTALL_PATH} \n"

for WORD in $(ls ..)
do
    if [ "$WORD" != "makefile" ]
    then
	cp -r ../${WORD} ${INSTALL_PATH}/
	if [ $? -ne 0 ]
	then 
	    echo -e "\nError en la instalación\n"
	    ERROR=1
	    break
	fi
	echo -e "\n    ${WORD} instalado \n"
    fi
done
trataERROR

ln -s ${INSTALL_PATH}/nucleo/shield.sh /usr/bin/shield.sh


echo '/usr/bin/shield.sh' >> /etc/shells

if [ $? -ne 0 ]
then
    ERROR=1
fi
trataERROR

echo -e "\nInstalación finalizada\n"

exit 0 
