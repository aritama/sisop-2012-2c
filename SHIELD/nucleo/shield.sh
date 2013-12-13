#!/bin/bash
set -m  # debe haber control de trabajo
set ignoreeof
#stty eof '?'

export shieldPID=$$

trap 'echo -e -n "\n[Shield] $(whoami)@${HOSTNAME}:${PWD}$ "' INT
trap 'echo -e -n "\n[Shield] $(whoami)@${HOSTNAME}:${PWD}$ "' SIGTSTP

shopt -s expand_aliases
function salirFunc
{
    for jobPid in `jobs -p`
    do 
        kill -KILL $jobPid
    done
    kill -KILL $shieldPID
}
alias salir='salirFunc'
alias exit='return'

export ORIGINAL_PATH=$(readlink -f /usr/bin/shield.sh)
ORIGINAL_PATH=${ORIGINAL_PATH%/*}
export SHIELD_USERS="$(cat ${ORIGINAL_PATH}/ShieldUsers)"
SHIELD_USERS="$(echo ${SHIELD_USERS} | tr '\n' ' ')"
export ShieldUser
for ShieldUser in $SHIELD_USERS
do
    if [ "$(whoami)" == "$ShieldUser" ]; then break; fi;
done
if [ "$(whoami)" != "$ShieldUser" ]
then
    echo -e "\n¡¡Usted no es usuario configurado de shield!!\n";
    
    salir
fi

function registrar
{
    export CANT_COMANDOS=0
    export CANT_PERIOD=0
#    export CANT_MOD_CONFS=0
    export CONFIG_COMANDOS="$(cat ~/.shield/comandos.config)"   
    export CONFIG_PERIOD="$(cat ~/.shield/periodicos.config)"
    export CONFIG_NUCLEO="$(cat ~/.shield/NUCLEO_CONFIG/nucleo.period)"
    export CONFIG_QUEUE="$(echo ${CONFIG_COMANDOS} | tr '\n' ' ')"

    ### 
    export CANT_MOD_CONFS=0
    export MOD_CONFS_PATH="/home/$(whoami)/.shield/MODULES_CONFIG"
    local list_mod_confs=$(ls $MOD_CONFS_PATH)
    for line in $list_mod_confs
    do
	CANT_MOD_CONFS=$((${CANT_MOD_CONFS} + 1))
	VEC_MOD_CONFS[${CANT_MOD_CONFS}]="$(cat "${MOD_CONFS_PATH}/${line}")"
    done
    
    
    for line in $CONFIG_QUEUE
    do
	if [ "${line##*:}" == "on" ]
	then 
	    CANT_COMANDOS=$((${CANT_COMANDOS}+1))
	    export VEC_COMANDOS_${CANT_COMANDOS}="${line%:*}"
	fi
    done 
    export CONFIG_QUEUE="$(echo ${CONFIG_PERIOD} | tr '\n' ' ')"
    for line in $CONFIG_QUEUE
    do
	if [ "${line##*:}" == "on" ]
	then
            CANT_PERIOD=$((${CANT_PERIOD}+1))
            export VEC_PERIOD_${CANT_PERIOD}="${line%:*}"
	fi
    done 
}

function inicializar
{
    for dir in ${!VEC_COMANDOS_*}
    do
  	source ${!dir} iniciar
	if [ $? -ne 0 ] # && [ -n "$(echo ${!dir} | grep modulo_control_sesiones.sh)" ]
	then
	    if [ -n "$(echo ${!dir} | grep modulo_control_sesiones.sh)" ] 
	    then
		echo -e "\n    Usted superó la máxima cantidad de sesiones admitidas\n"
		echo -e "$(date)\nMódulo de control de sesiones, máxima cantidad de sesiones admitidas superadas\n" >> ~/.shield/shell.log
		salir
	    else
		echo -e "\n    Error iniciando el módulo ${!dir##*/} \n"
		echo -e "$(date)\nError iniciando el módulo ${!dir##*/}\n" >> ~/.shield/shell.log
		salir
	    fi
	fi
    done
    for dir in ${!VEC_PERIOD_*}
    do
        source ${!dir} iniciar
	if [ $? -ne 0 ]
	then
	    echo -e "\n    Error iniciando el módulo ${!dir##*/} \n"
            echo -e "$(date)\nError iniciando el módulo ${!dir##*/}\n" >> ~/.shield/shell.log
	    salir
	fi
    done
}

function unsetearVecs
{
    for i in `seq 1 ${CANT_COMANDOS}`
    do
	unset VEC_COMANDOS_${i}
    done 
    for i in `seq 1 ${CANT_PERIOD}`
    do
	unset VEC_PERIOD_${i}
    done
#    for i in `seq 1 ${CANT_MOD_CONFS}`
#    do
	unset VEC_MOD_CONFS
#    done
}

function detenerModulos
{
    for dir in ${!VEC_COMANDOS_*}
    do
        source ${!dir} detener
    done
    for dir in ${!VEC_PERIOD_*}
    do
        source ${!dir} detener
    done
}

function verificarYrealizarCambios 
{
sleep 7
     while [ true ]
    do
	sleep $1
	if [ "${CONFIG_COMANDOS}" != "$(cat ~/.shield/comandos.config)" ] || [ "${CONFIG_PERIOD}" != "$(cat ~/.shield/periodicos.config)" ]
	then
#	    echo -e "\n Cambios detectados de los archivos de configuracion\nIntente no ingresar comandos. Podría producirse un error si justo acaba de ingresar uno\n"
	    kill -USR2 ${shieldPID}
	    continue
	fi
	if [ "${CONFIG_NUCLEO}" != "$(cat ~/.shield/NUCLEO_CONFIG/nucleo.period)" ]
	then 
#	    echo -e "\n Cambios detectados de los archivos de configuracion\nIntente no ingresar comandos. Podría producirse un error si justo acaba de ingresar uno\n"
	    kill -USR2 ${shieldPID}
	    continue
	fi

	local imodconf=0
	declare -a vec_modconf_loc
	local listAux=$(ls ${MOD_CONFS_PATH})
	for line in $listAux
	do
	    imodconf=$(($imodconf + 1))
	    vec_modconf_loc[${imodconf}]="$(cat ${MOD_CONFS_PATH}/${line})"
	done
	for line in `seq 1 ${imodconf}`
	do
	    if [ "${vec_modconf_loc[${line}]}" != "${VEC_MOD_CONFS[${line}]}" ]
	    then
		kill -USR2 ${shieldPID}
		break
	    fi
	done
    done
}

function leerPeriodosConfig
{
    export PERIODO_CAMBIOS=$(cat ~/.shield/NUCLEO_CONFIG/nucleo.period | grep "cambios" | grep "conf")
    if [ "${PERIODO_CAMBIOS}" == "${PERIODO_CAMBIOS##*= }" ]
    then
	PERIODO_CAMBIOS=${PERIODO_CAMBIOS##*=}
    else
	PERIODO_CAMBIOS=${PERIODO_CAMBIOS##*= }
    fi
    export PERIODO_MODLS=$(cat ~/.shield/NUCLEO_CONFIG/nucleo.period | grep "modulos periodicos")
    if [ "${PERIODO_MODLS}" == "${PERIODO_MODLS##*= }" ]
    then
	PERIODO_MODLS=${PERIODO_MODLS##*=}
    else
	PERIODO_MODLS=${PERIODO_MODLS##*= }
    fi
}

function getVerifCambJob
{
    local trabajo=$(echo "$(jobs)" | grep 'verificarYrealizarCambios ${PERIODO_CAMBIOS}')
    trabajo=${trabajo%]*}; trabajo=${trabajo##*[}
    echo $trabajo
}

function getModPerJob
{
    local trabajo=$(echo "$(jobs)" | grep 'ejecutarModulosPeriodicos ${PERIODO_MODLS}')
    trabajo=${trabajo%]*}; trabajo=${trabajo##*[}
    echo $trabajo
}





function ejecutarModulosPeriodicos 
{
    while [ true ]
    do
	sleep $1
	kill -USR1 ${shieldPID}
    done
} 


function apagamosLaCompu
{
    sudo poweroff
    salir
}
alias apagar="apagamosLaCompu"
function iniciarShield
{
    registrar
    leerPeriodosConfig
    inicializar
    ejecutarModulosPeriodicos ${PERIODO_MODLS} &
    verificarYrealizarCambios ${PERIODO_CAMBIOS} &
}
function ActualizarModulos
{
    kill -KILL %$(getVerifCambJob)
    kill -KILL %$(getModPerJob)
    detenerModulos
    unsetearVecs
    iniciarShield
}
alias actualizar_modulos="ActualizarModulos"
function comando_ayuda
{
    if [[  ${#1} -eq 0 ]]
    then
        echo -e "	info_modulos)
		informacion de modulo; ejemplo: info_modulos seguridad
	listar_modulos)
		Listar todos los modulos
	actualizar_modulos)
                actualizar modulos
        mostrar)
                Escribe un string o el contenido de una variable por la salida estandar\n		Modo de uso: mostrar variable/string
        salir)
                sale de Shield
        apagar)
                Apagar la maquina    
	ayuda) 
                brinda ayuda acerca de los bultins; uso: ayuda nombreDeBuiltin\n"
    else
        case $1 in
	    ayuda)
		echo "brinda ayuda acerca de los bultins; uso: ayuda nombreDeBuiltin";;
            info_modulos)
                echo "informacion de modulos";;
            listar_modulos)
                echo  "Listar todos los modulos";;
            actualizar_modulos)
                echo  "actualizar modulos";;
            mostrar)
                echo -e "Escribe un string o el contenido de una variable por la salida estandar\nModo de uso: mostrar variable/string";;
            salir)
                echo "sale de Shield";;
            apagar)
                echo "Apagar la maquina";;
            *)
		echo "Error shield:Parametro desconocido";;
        esac
    fi
}
alias ayuda="comando_ayuda"
function informar_modulos
{
    local moduloAct
    local auxModInfo=""
    local foundInformativeData=0
    if [[  ${#1} -eq 0 ]]
    then
        for dir in ${!VEC_COMANDOS_*}
        do
            source ${!dir} informacion
        done
	for dir in ${!VEC_PERIOD_*}
        do
            source ${!dir} informacion
        done
	return
    else
        for dir in ${!VEC_COMANDOS_*}
        do
            auxModInfo=$(echo ${!dir} | grep $1)
            if [ "${auxModInfo}" != "" ]
            then
		source ${!dir} informacion
	    fi
	    if [ "${auxModInfo}" != "" ] && [ $foundInformativeData -eq 0 ]
	    then
		foundInformativeData=1
	    fi
        done
	for dir in ${!VEC_PERIOD_*}
        do
	    auxModInfo=$(echo ${!dir} | grep $1)
            if [ "${auxModInfo}" != "" ]
            then
		source ${!dir} informacion
	    fi
	    if [ "${auxModInfo}" != "" ] && [ $foundInformativeData -eq 0 ]
	    then
		foundInformativeData=1
	    fi
	done
    fi
    if [ $foundInformativeData -eq 0 ]
    then
	echo -e "\nMódulo no reconocido\n"
    fi
}
alias info_modulos="informar_modulos"
function ListarModulos
{
    for dir in ${!VEC_COMANDOS_*}
    do
        echo ${!dir}
    done
    for dir in ${!VEC_PERIOD_*}
    do
        echo ${!dir}
    done
}
alias listar_modulos="ListarModulos"
                                                                                                                                
function mostrar_variable
{
   echo $1
}
alias mostrar="mostrar_variable"



export INPUT_USER_SHIELD=''


function executePeriodicos
{
    local ErrorOnPeriod=0
    local ModuleErrorPeriod=''
    for dir in ${!VEC_PERIOD_*}                                                                                                                    
    do  
	source ${!dir} procesar
	if [ $? -ne 0 ]; then ErrorOnPeriod=1; ModuleErrorPeriod="${!dir}"; break; fi;  
    done  
    
    if [ $ErrorOnPeriod -ne 0 ]
    then 
	echo -e "Error relacionado con ${ModuleErrorPeriod##*/}, se procedera a finalizar la sesión\n"
	echo -e "$(date)\nError relacionado con ${ModuleErrorPeriod##*/}, modulo periodico\n" >> ~/.shield/shell.log
	salir
    fi
}

function trapUSR1
{
    executePeriodicos
}

function executeActualizar
{
    CONFIG_COMANDOS="$(cat ~/.shield/comandos.config)"
    CONFIG_PERIOD="$(cat ~/.shield/periodicos.config)"
    CONFIG_NUCLEO="$(cat ~/.shield/NUCLEO_CONFIG/nucleo.period)"
    ActualizarModulos
    echo -e "\n    Cambios de los archivos de configuración aplicados\n"
    echo -e -n "\n[Shield] $(whoami)@${HOSTNAME}:${PWD}$ "
}


function trapUSR2 # verificar cambios y realizarlos
{
#    echo -e "\nSe actualizaran las configuraciones luego de la ejecución de su siguiente comando\n"
    echo -e -n "\n[Shield] $(whoami)@${HOSTNAME}:${PWD}$ "
    executeActualizar
}

trap "trapUSR1" SIGUSR1
trap "trapUSR2" SIGUSR2

iniciarShield

export HUBO_ERROR=0
export ERROR_MODULO=''

echo -e "\n Bienvenid@ a Shield \n"

while [ "${INPUT_USER_SHIELD}" != "salir" ] && [ "${INPUT_USER_SHIELD}" != "exit" ]
do
    read -e -p "[Shield] $(whoami)@${HOSTNAME}:${PWD}$ " INPUT_USER_SHIELD
    for dir in ${!VEC_COMANDOS_*}
    do
	if [ "$(echo ${!dir} | grep "modulo_control_sesiones.sh")" != "" ]
	then
	  source ${!dir} iniciar
	else
	  source ${!dir} procesar "${INPUT_USER_SHIELD}"
	fi
	if [[ $? -ne 0 ]]
	then
	    HUBO_ERROR=1
	    ERROR_MODULO=${!dir}
        fi
    done
    
    if [[ ${HUBO_ERROR} -ne 1 ]]
    then 
	if [ "${INPUT_USER_SHIELD}" != "salir" ] && [ "${INPUT_USER_SHIELD}" != "exit" ]
	then
	    eval ${INPUT_USER_SHIELD}
	fi
    else
	echo -e "\nError relacionado con ${ERROR_MODULO##*/}\n"
	echo -e "$(date)\nError relacionado con ${ERROR_MODULO##*/}\ninput: ${INPUT_USER_SHIELD}\n" >> ~/.shield/shell.log
	HUBO_ERROR=0
	ERROR_MODULO=''
    fi

done

kill -KILL %$(getVerifCambJob)
kill -KILL %$(getModPerJob)
#eval ${INPUT_USER_SHIELD} 0
salir
