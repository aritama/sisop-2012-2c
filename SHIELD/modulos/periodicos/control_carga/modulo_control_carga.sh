#!/bin/bash


if [ $1 = "iniciar" ];
then
	max_CPU=$(cat ~/.shield/MODULES_CONFIG/control_carga.config)
	
	export MAX_CPU=$max_CPU
	#echo $MAX_CPU #prueba
	export PIDULTIMOAUMENENTADO=0
	#echo $PIDULTIMOAUMENENTADO #prueba
	export CANT_INCREMENTOS=0
	#echo $CANT_INCREMENTOS #prueba
	export NICEACTUAL=0
	#echo $NICEACTUAL #prueba
	export ULTIMO_MATADO=0
	#echo $ULTIMO_MATADO #prueba
	return 0
fi

#--------------------------------------------------------------------------------

if [ $1 = "detener" ];
then
  unset MAX_CPU
  unset PIDULTIMOAUMENENTADO
  unset CANT_INCREMENTOS
  unset NICEACTUAL
  unset ULTIMO_MATADO
 return 0
fi

#--------------------------------------------------------------------------------

if  [ $1 = "informacion" ];
then

if [ -a /proc/$PIDULTIMOAUMENENTADO ] 
then

    echo -e "\nEl ultimo proceso afectado fue: $PIDULTIMOAUMENENTADO"
    echo "MAX CPU : ${MAX_CPU}"
    echo "Su nice Actual es:" $NICEACTUAL
    echo -e "Y fue afectado" $CANT_INCREMENTOS "veces\n"
    
else
    
    echo -e "\nMAX CPU: ${MAX_CPU}"
    echo -e "Actualmente no hay procesos afectados por el modulo de control de carga\n"
    
fi
return 0
fi
#--------------------------------------------------------------------------------

if  [ $1 = "procesar" ]; then

OUT=$(mktemp /tmp/output.XXXXXXXXXX) || { echo "No se pudo abrir archivo temporal"; exit 1; }

ps -o pid,pcpu,nice --sort pcpu | grep -v -w shield | grep -v ps | grep -v grep | tail -1 > $OUT
pidMayor=$(awk  '{print $1}' $OUT)
usoCPU=$(awk  '{print $2}' $OUT)
niceActual=$(awk  '{print $3}' $OUT)
#echo $pidMayor
#echo $usoCPU
#echo $niceActual


	if [ $(echo "$MAX_CPU < $usoCPU"|bc) -eq 1 ];
	then
		nuevoNice=$(($niceActual + 5))	
		
		#echo $nuevoNice
		renice $nuevoNice $pidMayor
		NICEACTUAL=$nuevoNice
		if [ $pidMayor -eq $PIDULTIMOAUMENENTADO ];
		then
			CANT_INCREMENTOS=$(( $CANT_INCREMENTOS + 1 ))
			if [ $CANT_INCREMENTOS -eq 4 ];
			then
			kill $PIDULTIMOAUMENENTADO
			PIDULTIMOAUMENENTADO=0
			CANT_INCREMENTOS=0
			NICEACTUAL=0
			ULTIMO_MATADO=$pidMayor
			fi
		else
			CANT_INCREMENTOS=1
			PIDULTIMOAUMENENTADO=$pidMayor
			NICEACTUAL=$nuevoNice
		fi
	fi
rm $OUT


return 0
fi

#--------------------------------------------------------------------------------

echo $1 no es un parametro esperado
echo "Los parametros aceptados son iniciar, informacion, detener y procesar."
return 0

