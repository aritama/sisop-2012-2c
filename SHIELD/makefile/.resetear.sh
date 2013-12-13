#!/bin/bash

if [ "$(whoami)" != "root" ]
then
    echo -e "\nSolo usuario con privilegios root debe resetear\n"
    exit 1
fi

if [ ! -h /usr/bin/shield.sh ]; then echo -e "\nShield no se encuentra instalado\n"; exit 1; fi;

ORIGINAL_PATH=$(readlink -f /usr/bin/shield.sh)
ORIGINAL_PATH=${ORIGINAL_PATH%/*}
ORIGINAL_PATH=${ORIGINAL_PATH%/*}

if [ ! -f ${ORIGINAL_PATH}/nucleo/ShieldUsers ]; then echo -e "\nNo hay usuarios configurados para resetear\n"; exit 1 ; fi;
if [ -z "$(cat ${ORIGINAL_PATH}/nucleo/ShieldUsers)" ]; then echo -e "\nNo hay usuarios configurados para resetear\n"; exit 1 ; fi;

echo "Usuarios configurados: "
CONFIGURADOS="$(cat ${ORIGINAL_PATH}/nucleo/ShieldUsers | tr '\n' ' ')"
for Usuario in ${CONFIGURADOS}
do
    echo "    ${Usuario}"
done
echo -e "\\n"

read -e -p "Ingrese nombre del usuario: " USER_NAME
USER_DATA=$(cat ${ORIGINAL_PATH}/.configured_users | grep $USER_NAME)
if [ -z "$USER_DATA" ]; then echo -e "\nNo está configurado el usuario $USER_NAME\nNo se puede resetar\n"
    exit 1
fi;
USER_DATA=$(echo $USER_DATA | tr ':' ' ')
function reinstaurarShell
{
    chsh -s $2 $1
    echo -e "\nReinstaurado $2 al usuario ${1}\n"
}
reinstaurarShell $USER_DATA

USER_DATA=$(echo $USER_DATA | tr ' ' ':')

  
sed -i "s|${USER_DATA}||g" ${ORIGINAL_PATH}/.configured_users
sed -i '/^$/d' ${ORIGINAL_PATH}/.configured_users

poweroffOff=$(cat /etc/sudoers | grep ${USER_NAME} | grep poweroff | grep shield)

sed -i "s|${poweroffOff}||g" /etc/sudoers ##

chshOff=$(cat /etc/sudoers | grep $USER_NAME | grep 'chsh' | grep shield)

sed -i "s|${chshOff}||g" /etc/sudoers ##


sed -i '/^$/d' /etc/sudoers

rm -f -r /home/${USER_NAME}/.shield

sed -i "s|${USER_NAME}||g" ${ORIGINAL_PATH}/nucleo/ShieldUsers
sed -i '/^$/d' ${ORIGINAL_PATH}/nucleo/ShieldUsers
echo -e "\nUsuario ${USER_NAME} reseteado\n"

exit 0
