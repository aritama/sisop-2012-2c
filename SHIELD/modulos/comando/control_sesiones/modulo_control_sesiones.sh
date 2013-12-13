#!/bin/bash

function iniciar
{
    MAX_SESSIONS_PER_USER=$(cat ~/.shield/MODULES_CONFIG/control_sesiones.config | grep "MAXIMA")
    if [ "${MAX_SESSIONS_PER_USER}" == "${MAX_SESSIONS_PER_USER##*= }" ]
    then
	MAX_SESSIONS_PER_USER=${MAX_SESSIONS_PER_USER##*=}
    else
	MAX_SESSIONS_PER_USER=${MAX_SESSIONS_PER_USER##*= }
    fi

    USER_ACTUAL_SESSIONS=$(who | grep -w -c $(whoami))

    if [[ $USER_ACTUAL_SESSIONS -gt $MAX_SESSIONS_PER_USER ]]
    then
	return 1
    else
	return 0
    fi
}

function informacion
{
    echo -e "\n$(cat ~/.shield/MODULES_CONFIG/control_sesiones.config)"
    echo -e "Cantidad de sesiones abiertas por el usuario $(whoami): $(who | grep -w -c $(whoami))\n"
    return 0
}

function detener
{
    unset MAX_SESSIONS_PER_USER
    unset USER_ACTUAL_SESSIONS
    return 0
}

case "$1" in
    "iniciar")
	iniciar;;
    "informacion")
	informacion;;
    "detener")
	detener;;
esac
