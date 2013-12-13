#!/bin/bash

function iniciar
{
    export RESTRICTED_COMMANDS=$(cat ~/.shield/MODULES_CONFIG/seguridad.config | tr '\n' ' ')
    return 0
}

function procesar
{
    local seguError=0
    for WORD in ${RESTRICTED_COMMANDS}
    do
	if [ -n "$(echo "$1" | grep $WORD)" ]
	then
	    seguError=1
	fi
    done
    return $seguError
}

function informacion
{
    echo -e "\nLos comandos restringidos son:"
    echo -e "$(echo ${RESTRICTED_COMMANDS} | tr ' ' '\n')\n"
    return 0
}

function detener 
{
    unset RESTRICTED_COMMANDS
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
