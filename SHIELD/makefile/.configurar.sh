#!/bin/bash

if [ "$(whoami)" != "root" ]
then
    echo -e "\nSolo usuario con privilegios root debe configurar\n"
    exit 1
fi

if [ ! -h /usr/bin/shield.sh ]; then echo -e "\nShield no se encuentra instalado\n"; exit 1; fi;

ORIGINAL_PATH=$(readlink -f /usr/bin/shield.sh)
ORIGINAL_PATH=${ORIGINAL_PATH%/*}
ORIGINAL_PATH=${ORIGINAL_PATH%/*}

read -e -p "Ingrese nombre del usuario a configurar: " USER_NAME

if [ -z $(cat /etc/shadow | grep ${USER_NAME}) ]; then echo -e "\nNo existe el usuario $USER_NAME  en el sistema\n"; exit 1; fi;

if [ -f ${ORIGINAL_PATH}/.configured_users ]
then
    if [ -n "$(cat ${ORIGINAL_PATH}/.configured_users | grep ${USER_NAME})" ]
    then 
	echo -e "\nEl usuario ya se encuentra configurado\n"
	exit 1
    fi
fi

if [ ! -d /home/${USER_NAME} ]
then
    mkdir /home/${USER_NAME}
    chown ${USER_NAME} /home/${USER_NAME}
    chgrp ${USER_NAME} /home/${USER_NAME}
fi
if [ ! -d /home/${USER_NAME}/.shield ]
then 
    mkdir /home/${USER_NAME}/.shield
#    chown ${USER_NAME} /home/${USER_NAME}/.shield
#    chgrp ${USER_NAME} /home/${USER_NAME}/.shield
fi

if [ ! -d /home/${USER_NAME}/.ssh ]
then    
    mkdir /home/${USER_NAME}/.ssh
    chown ${USER_NAME} /home/${USER_NAME}/.ssh
    chgrp ${USER_NAME} /home/${USER_NAME}/.ssh
fi

Comandos_Path="/home/${USER_NAME}/.shield/comandos.config"
touch $Comandos_Path
Period_Path="/home/${USER_NAME}/.shield/periodicos.config"
touch $Period_Path


for WORD in $(ls ../modulos/*/*/*.sh)
do
    PART_DIR=${WORD##../}
    if [ -n "$(echo $PART_DIR | grep modulos/comando)" ]
    then
	if [ -n "$(echo $PART_DIR | grep modulos/comando | grep control_sesiones)" ]
	then 
	    echo "${ORIGINAL_PATH}/${PART_DIR}:on" >> $Comandos_Path
	else
	    echo "${ORIGINAL_PATH}/${PART_DIR}:off" >> $Comandos_Path
	fi
    fi
    if [ -n "$(echo $PART_DIR | grep modulos/periodicos)" ]
    then
	echo "${ORIGINAL_PATH}/${PART_DIR}:off" >> $Period_Path
    fi
done

mkdir /home/${USER_NAME}/.shield/NUCLEO_CONFIG/
cp ../nucleo/nucleo.period /home/${USER_NAME}/.shield/NUCLEO_CONFIG/

mkdir /home/${USER_NAME}/.shield/MODULES_CONFIG/

for WORD in $(ls ../*/*/*/*.config) # todos los .config van a MODULES_CONFIG del usuario
do
    cp ${WORD} /home/${USER_NAME}/.shield/MODULES_CONFIG/
done

export USER_DATA=$(cat /etc/passwd | grep $USER_NAME)

export USER_PREV_SHELL=${USER_DATA##*:}

echo "${USER_NAME}:${USER_PREV_SHELL}" >> ${ORIGINAL_PATH}/.configured_users

chsh -s /usr/bin/shield.sh $USER_NAME

echo "${USER_NAME} ${HOSTNAME}=NOPASSWD: /sbin/poweroff   #shield" >>  /etc/sudoers
echo "$USER_NAME ${HOSTNAME}=NOPASSWD: /usr/bin/chsh    #shield" >> /etc/sudoers
echo "${USER_NAME}" >> ${ORIGINAL_PATH}/nucleo/ShieldUsers

#chown -R root /home/${USER_NAME}/.shield
#chgrp -R root /home/${USER_NAME}/.shield
touch /home/${USER_NAME}/.shield/shell.log
touch /home/${USER_NAME}/.shield/input.log
chown $USER_NAME /home/${USER_NAME}/.shield/shell.log
chgrp $USER_NAME /home/${USER_NAME}/.shield/shell.log
chown $USER_NAME /home/${USER_NAME}/.shield/input.log
chgrp $USER_NAME /home/${USER_NAME}/.shield/input.log

chmod 644 /home/${USER_NAME}/.shield/shell.log
chmod 644 /home/${USER_NAME}/.shield/input.log

echo -e "\nSe pudo configurar al usuario $USER_NAME \n"

exit 0
