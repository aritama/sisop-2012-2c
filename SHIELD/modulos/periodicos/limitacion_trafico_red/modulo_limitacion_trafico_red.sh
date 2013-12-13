#!/bin/bash                                                                                                                                                  

function obtenerPaquetesEnviados
{
    local aux=0
    for file in $(ls /sys/class/net)
    do
        if [ "${file}" != "lo" ]
        then
            aux=$(($aux + $(cat /sys/class/net/${file}/statistics/tx_packets)))
        fi
    done
    PAQUETES_ENVIADOS=$aux
}

function iniciar
{
    eval $(cat ~/.shield/MODULES_CONFIG/limitacion_trafico_red.config | grep "LIMITE_PAQUETES")
#    LIMITE_PAQUETES=9000000000
    obtenerPaquetesEnviados
    return 0
}

#lsof -i4 -w | grep -v "COMMAND" | head -n 1 | awk '{print $2}'     pid ,, $1 nombre

function procesar 
{
    local processCount=0
    local name
    local cantSockets
    local listName
    local pid
    local listElim

    obtenerPaquetesEnviados
    if [ "$PAQUETES_ENVIADOS" -gt "$LIMITE_PAQUETES" ]
    then
	listName=$(lsof -i4 -w | grep -v "COMMAND" | grep -v "localhost" | awk '{print $1}' | uniq)
	for name in $listName
	do
	    cantSockets=$(lsof -i4 -w | grep -v "COMMAND" | grep -v "localhost" | awk '{print $1}' | grep -c ${name})

	    listElim=$(lsof -i4 -w | grep -v "COMMAND" | grep -v "localhost" | grep ${name} | awk '{print $2}')
	    
	    if [ $cantSockets -gt 1 ]
	    then
		echo -e "\nEl programa ${name}, posee $cantSockets sockets de red, se eliminaran\n"
		for pid in $listElim
		do
		    kill -KILL $pid 2> /dev/null
		done
	    fi
	done
    fi
    return 0
}

function informacion
{
    obtenerPaquetesEnviados
    echo -e "\nComienza limitación con la siguiente cantidad de paquetes enviados: $LIMITE_PAQUETES"
    echo -e "Paquetes que se han envíado: $PAQUETES_ENVIADOS\n"
    return 0
}

function detener
{
    unset PAQUETES_ENVIADOS
    unset LIMITE_PAQUETES
}

case "$1" in
    "iniciar")
	iniciar;;
    "procesar")
	procesar;;
    "informacion")
	informacion;;
    "detener")
	detener;;
esac
