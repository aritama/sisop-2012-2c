#!/bin/bash

function iniciar
{
    export INPUT_PROCESAR_AUDITORIA
    export IP_SERVER_INPUT_LOG=$(cat ~/.shield/MODULES_CONFIG/auditoria.config | grep "IP SERVER INPUT LOG")
    if [ "${IP_SERVER_INPUT_LOG}" == "${IP_SERVER_INPUT_LOG##*= }" ]
    then
	IP_SERVER_INPUT_LOG=${IP_SERVER_INPUT_LOG##*=}
    else
	IP_SERVER_INPUT_LOG=${IP_SERVER_INPUT_LOG##*= }
    fi
    #
    export MAX_INPUT_LOG_SIZE=$(cat ~/.shield/MODULES_CONFIG/auditoria.config | grep "MAX INPUT LOG SIZE")
    if [ "${MAX_INPUT_LOG_SIZE}" == "${MAX_INPUT_LOG_SIZE##*= }" ]
    then
	MAX_INPUT_LOG_SIZE=${MAX_INPUT_LOG_SIZE##*=}
    else
	MAX_INPUT_LOG_SIZE=${MAX_INPUT_LOG_SIZE##*= }
    fi
    #
    export ACTUAL_INPUT_LOG_SIZE=0
    if [ -f ~/.shield/input.log ]
    then
	if [ -w ~/.shield/input.log ]
	then
	    ACTUAL_INPUT_LOG_SIZE=$(stat -c%s ~/.shield/input.log)
	else
	    return 1
	fi
    else
	touch ~/.shield/input.log
    fi
    
    export PUBLIC_KEY_ALREADY_SENT="false"
    return 0
}
huboQueCambiarHostKeyChecking=0
function comprobarHostKeyChecking 
{
	if [ -f ~/.ssh/config ]
	then
		if [ -n "$(cat ~/.ssh/config | grep StrictHostKeyChecking)" ]
		then
			if [ -z "$(cat ~/.ssh/config | grep 'StrictHostKeyChecking no')" ]
			then
				sed -i 's|StrictHostKeyChecking yes|StrictHostKeyChecking no|g' ~/.ssh/config
				huboQueCambiarHostKeyChecking=1
			fi
		else
			echo 'StrictHostKeyChecking no' >> ~/.ssh/config
			huboQueCambiarHostKeyChecking=1
		fi
	else
		echo 'StrictHostKeyChecking no' >> ~/.ssh/config
                huboQueCambiarHostKeyChecking=1
	fi
}

function devolverValorHostCheck
{
	if [ $huboQueCambiarHostKeyChecking -ne 0 ]
	then
		sed -i 's|StrictHostKeyChecking no|StrictHostKeyChecking yes|g' ~/.ssh/config
		huboQueCambiarHostKeyChecking=0
	fi
}

function procesar
{
#    ACTUAL_INPUT_LOG_SIZE=$(stat -c%s ~/.shield/input.log)
    if [ ${ACTUAL_INPUT_LOG_SIZE} -ge ${MAX_INPUT_LOG_SIZE} ]
    then
	if [ -f ~/.ssh/id_rsa ] && [ -f ~/.ssh/id_rsa.pub ]
	then
#	    echo -e "\nEspacio maximo del log local alcanzado, se solicitara ingreso de contraseña del servidor, se generara clave publica en caso necesario\n"
	    if [ "${PUBLIC_KEY_ALREADY_SENT}" == "false" ]
	    then
		echo -e "\nEspacio maximo del log local alcanzado, se solicitara ingreso de contraseña del servidor y  se generara clave publica en casos necesarios\n"
		# ssh $(whoami)@${IP_SERVER_INPUT_LOG} "cat >> ~/.ssh/authorized_keys" < ~/.ssh/id_rsa.pub
		comprobarHostKeyChecking
		ssh-copy-id $(whoami)@${IP_SERVER_INPUT_LOG}
		ssh $(whoami)@${IP_SERVER_INPUT_LOG} "if [ ! -d /home/$(whoami) ]; then mkdir /home/$(whoami); fi;"
		ssh $(whoami)@${IP_SERVER_INPUT_LOG} "if [ ! -d /home/$(whoami)/.shield ]; then mkdir /home/$(whoami)/.shield; fi;"
		PUBLIC_KEY_ALREADY_SENT="true"
		devolverValorHostCheck
	    fi
	else
	    echo -e "\nEspacio maximo del log local alcanzado, se solicitara ingreso de contraseña del servidor y se generara clave publica en casos necesarios\n"
	    ssh-keygen -q -t rsa -f ~/.ssh/id_rsa -N ''
#	    ssh $(whoami)@${IP_SERVER_INPUT_LOG} "cat >> ~/.ssh/authorized_keys" < ~/.ssh/id_rsa.pub
	    comprobarHostKeyChecking
	    ssh-copy-id $(whoami)@${IP_SERVER_INPUT_LOG}
	    ssh $(whoami)@${IP_SERVER_INPUT_LOG} "if [ ! -d /home/$(whoami) ]; then mkdir /home/$(whoami); fi;"
	    ssh $(whoami)@${IP_SERVER_INPUT_LOG} "if [ ! -d /home/$(whoami)/.shield ]; then mkdir /home/$(whoami)/.shield; fi;"
	    PUBLIC_KEY_ALREADY_SENT="true"
	    devolverValorHostCheck
	fi
	echo "$1" | ssh $(whoami)@${IP_SERVER_INPUT_LOG} "cat >> ~/.shield/input.log"
    else
	echo "$1" >> ~/.shield/input.log
    fi
    ACTUAL_INPUT_LOG_SIZE=$(stat -c%s ~/.shield/input.log)
    return 0
}

function informacion
{
    echo -e "\n$(cat ~/.shield/MODULES_CONFIG/auditoria.config)"
    echo -e "Tamaño actual del archivo de log local: ${ACTUAL_INPUT_LOG_SIZE}\n"
    return 0
}

function detener
{
    unset IP_SERVER_INPUT_LOG
    unset MAX_INPUT_LOG_SIZE
    unset ACTUAL_INPUT_LOG_SIZE
    unset PUBLIC_KEY_ALREADY_SENT
    return 0
}

case "$1" in
    "iniciar")
	iniciar;;
    "procesar")
	procesar "$2";;
    "informacion")
	informacion;;
    "detener")
	detener;;
esac
