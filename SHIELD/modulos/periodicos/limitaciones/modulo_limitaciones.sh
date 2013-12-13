#!/bin/bash

function iniciar
{
    export MAX_CPU=50
    export MAX_MEM=75
    export MAX_PROCS=10
    export MAX_SOCKS=3
    export MAX_ARCHS=4
    local limitacionesInit="$(cat ~/.shield/MODULES_CONFIG/limitaciones.config)"
    eval "${limitacionesInit}"
    ###
    local listaConsumoCPU=$(ps h u t | awk '{print $3}')
    consumoCPU=0
    for consumo in $listaConsumoCPU
    do
	consumoCPU=$(echo "${consumoCPU} + ${consumo}" | bc -l)
    done
    ###
    local listaConsumoMEM=$(ps h u t | awk '{print $4}')
    consumoMEM=0
    for consumo in $listaConsumoMEM
    do
        consumoMEM=$(echo "${consumoMEM} + ${consumo}" | bc -l)
    done
    ###
    consumoPROCS=$(($(ps h u t | wc -l) - 5)) ## sin contar procesos de 1)login,2)bash,3)ps, 4)wc,5)asignacion,,sin contar los que ya estaban
    ###
    local listaPIDS=$(ps h u t | awk '{print $2}' | uniq)
    local listaSocketsPids=$(lsof -i4 | grep TCP | grep -v "localhost" | awk '{print $2}' | uniq)
    consumoSOCKS=0
    for limitPid in $listaPIDS
    do
	for limitPid2 in $listaSocketsPids
	do
	    if [ $limitPid -eq $limitPid2 ]
	    then
		consumoSOCKS=$(($consumoSOCKS + 1))
	    fi
	done
    done
    ###
    local mytty=`tty`
    consumoARCHS=0
    local listTtyPids=$(lsof $mytty | awk '{print $2}' | uniq)
    for pid in $listTtyPids
    do
	auxConsArchs="$(ls "/proc/${pid}/fd" 2> /dev/null | tr "$IFS" '\n')"
	auxConsArchs="$(echo "${auxConsArchs}" | grep -v 0 | grep -v 1 | grep -v 2 | grep -v 3 | grep -v 255)"
	if [ "$auxConsArchs" == "" ]
	then 
	    auxConsArchs=0
	else
            auxConsArchs=$(echo "${auxConsArchs}" | wc -l)
	fi
	consumoARCHS=$(($consumoARCHS + ${auxConsArchs}))
    done

    return 0
}

function detener
{
    unset MAX_CPU
    unset MAX_MEM
    unset MAX_PROCS
    unset MAX_SOCKS
    unset MAX_ARCHS
    return 0
}

function procesar
{
    local listaConsumoCPU=$(ps h u t | awk '{print $3}')
    consumoCPU=0
    for consumo in $listaConsumoCPU
    do
	consumoCPU=$(echo "${consumoCPU} + ${consumo}" | bc -l)
    done
    if [ $(echo "${consumoCPU} > ${MAX_CPU}" | bc -l) -eq 1 ]
    then
	return 1
    fi
    ###
    local listaConsumoMEM=$(ps h u t | awk '{print $4}')
    consumoMEM=0
    for consumo in $listaConsumoMEM
    do
        consumoMEM=$(echo "${consumoMEM} + ${consumo}" | bc -l)
    done
    if [ $(echo "${consumoMEM} > ${MAX_MEM}" | bc -l) -eq 1 ]
    then
	return 1
    fi
    ###
    consumoPROCS=$(($(ps h u t | wc -l) - 9)) ## sin contar procesos de 1)login,2)bash,3)ps, 4)wc,5)asignacion,,sin contar los que ya estaban
    if [ $(echo "${consumoPROCS} > ${MAX_PROCS}" | bc -l) -eq 1 ]
    then
	return 1
    fi
    ###
    local listaPIDS=$(ps h u t | awk '{print $2}' | uniq)
    local listaSocketsPids=$(lsof -i4 | grep TCP | grep -v "localhost" | awk '{print $2}' | uniq)
    consumoSOCKS=0
    for limitPid in $listaPIDS
    do
	for limitPid2 in $listaSocketsPids
	do
	    if [ $limitPid -eq $limitPid2 ]
	    then
		consumoSOCKS=$(($consumoSOCKS + 1))
	    fi
	done
    done
    if [ $consumoSOCKS -gt $MAX_SOCKS ]
    then
	return 1
    fi
    ###
    local mytty=`tty`
    consumoARCHS=0
    local listTtyPids=$(lsof $mytty | awk '{print $2}' | uniq)
    for pid in $listTtyPids
    do
	auxConsArchs="$(ls "/proc/${pid}/fd" 2> /dev/null | tr "$IFS" '\n')"
	auxConsArchs="$(echo "${auxConsArchs}" | grep -v 0 | grep -v 1 | grep -v 2 | grep -v 3 | grep -v 255)"
	if [ "$auxConsArchs" == "" ]
	then 
	    auxConsArchs=0
	else
            auxConsArchs=$(echo "${auxConsArchs}" | wc -l)
	fi
	consumoARCHS=$(($consumoARCHS + ${auxConsArchs}))
    done
    #consumoARCHS=$(($(lsof $mytty | awk '{print $2}' | uniq | wc -l) - 11)) # menos lsof,awk,uniq,wc,asig,bash
    if [ $consumoARCHS -gt $MAX_ARCHS ]
    then
	return 1
    fi
    
    return 0
}

function informacion
{
    echo -e "\nMAX_CPU: ${MAX_CPU} ;  consumoCPU: ${consumoCPU}"
    echo "MAX_MEM: ${MAX_MEM} ;  consumoMEM: ${consumoMEM}"
    echo "MAX_SOCKS: ${MAX_SOCKS} ;  consumoSOCKS: ${consumoSOCKS}"
    echo "MAX_PROCS: ${MAX_PROCS} ;  consumoPROCS: ${consumoPROCS}"
    echo -e "MAX_ARCHS: ${MAX_ARCHS} ;  consumoARCHS: ${consumoARCHS}\n"
    return 0
}

case "$1" in
    "iniciar")
	iniciar
	;;
    "detener")
	detener
	;;
    "procesar")
	procesar
	;;
    "informacion")
	informacion
	;;
esac
