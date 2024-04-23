#define GET_LINE NO_LINE

/* Fichero con funciones específicas para la lectura del fichero de configuración. Reciben un Globals*.

Leen un dato y muestran el mensaje de aviso o error.
*/

/* err:			Contiene el error que se produjo al intentar leer un valor del tipo indicado en el nombre de la función.
errors:		Para escribir el mensaje de error e incrementar err_count o war_count.
file:			Nombre del fichero en que se leyó la clave. Puede ser NULL
key:			El nombre de la clave.
str:			Lo que se leyó del fichero tal cual.
sval:			Si err>=0, es la medida en décimas de milímetro.
max_val:		Máximo valor permitido
max_recom:	Máximo valor esperado.

Si err= 0 la función no hace nada.
Si err!=0 Incrementa err_count o war_count.
*/

void error_Bool_err(int err, ErrorOpts *errors, const char8_t *file, uint line, const char8_t* key, const char8_t *str){
	ifz(err) return;
	if(err<0){iferrwar(errors,LERR); else goto done;}
	else{iferrwar(errors,LERR); else goto done;}

	if(err==ERR_VAL_EMPTY) Write_ERROR(errors, u8"No se puede convertir el valor de %s a bool porque es una cadena vacía.",key);
	elif(err==ERR_VAL_NOBOOLEAN) Write_ERROR(errors, u8"Al intentar convertir el valor de %s a bool. Se ha indicado algo que no es un valor booleano: %SB.", key,str);
	else Write_ERROR(errors, u8"Error desconocido al intentar convertir el valor de %s a bool: %SB.", key,str);
	if(file!=NULL) LOG_f(errors,NO_LINE,u8"    Clave leída en el archivo %s, línea %u\n",file,line);

done:
	if(err<0) errors->err_count++;
	else errors->war_count++;
}

void error_Entero_err(int err, ErrorOpts *errors, const char8_t *file, uint line, const char8_t* key, const char8_t *str, uint _unused(val), uint max_val, uint _unused(max_recom)){
	ifz(err) return;
	if(err<0){iferrwar(errors,LERR); else goto done;}
	else{iferrwar(errors,LERR); else goto done;}

	if(err==ERR_VAL_EMPTY) Write_ERROR(errors, u8"No se puede convertir el valor de %s a un entero porque es una cadena vacía.",key);
	elif(err==ERR_VAL_NONUMBER) Write_ERROR(errors, u8"Al intentar convertir el valor de %s a entero. Se ha indicado algo que no es un número: %SB.", key,str);
	elif(err==ERR_VAL_NUMHUGE || err==ERR_VAL_HUGE) Write_ERROR(errors, u8"Al intentar convertir el valor de %s a entero. El valor máximo posible es %u. Se ha indicado %SB.", key,Я,str);
	elif(err==ERR_VAL_RANGE) Write_ERROR(errors, u8"En la conversión del valor de %s a entero. El valor máximo permitido es %u. Se ha indicado %SB.", key,max_val,str);
	elif(err==WAR_VAL_RANGE) Write_WARNING(errors, u8"Valor anormalmente alto en la calve %s. Se encontró %SB.", key,str);
	else Write_ERROR(errors, u8"Error desconocido al intentar convertir el valor de %s a un entero: %SB.", key,str);
	if(file!=NULL) LOG_f(errors,NO_LINE,u8"    Clave leída en el archivo %s, línea %u\n",file,line);

done:
	if(err<0) errors->err_count++;
	else errors->war_count++;
}

void error_Magnitud_err(int err, ErrorOpts *errors, const char8_t *file, uint line, const char8_t* key, const char8_t *str, bint bneg, uint val, uint max_val, uint _unused(max_recom), const char8_t *uni){
	ifz(err) return;
	if(err<0){iferrwar(errors,LERR); else goto done;}
	else{iferrwar(errors,LERR); else goto done;}

	if(err==ERR_VAL_EMPTY) Write_ERROR(errors, u8"No se puede convertir el valor de %s a una magnitud porque es una cadena vacía.",key);
	elif(err==ERR_VAL_NONUMBER) Write_ERROR(errors, u8"Al intentar convertir el valor de %s a una magnitud. Se ha indicado algo que no es un número: %s.", key,str);
	elif(err==ERR_VAL_NUMHUGE) Write_ERROR(errors, u8"Al intentar convertir el valor de %s a una magnitud. Se ha indicado en el fichero un número mayor que " MAX_MAGNI_VAL_String ".",key);
	elif(err==ERR_UNI_ABSENT) Write_ERROR(errors, u8"Al intentar convertir el valor de %s a una magnitud. Faltan las unidades", key);
	elif(err==ERR_UNI_WRONG) Write_ERROR(errors, u8"Al intentar convertir el valor de %s a una magnitud. Se han indicado unas unidades erróneas: %s.", key,str);
	elif(err==ERR_VAL_HUGE) Write_ERROR(errors, u8"Al intentar convertir el valor de %s a una magnitud. El valor máximo posible es %u %s. Se ha indicado %s.", key,0xFFFFffff,uni,str);
	elif(err==ERR_VAL_NEG) Write_WARNING(errors, u8"En la conversión del valor de %s a una magnitud. En %s no se permiten valores negativos", key,key);
	elif(err==ERR_VAL_RANGE) Write_ERROR(errors, u8"En la conversión del valor de %s a una magnitud. El valor máximo permitido es %u %s. Se ha indicado %s.", key,max_val,uni,str);
	elif(err==WAR_VAL_RANGE) Write_WARNING(errors, u8"Valor anormalmente alto en la clave %s. Se encontró %s.", key,str);
	elif(err==WAR_VAL_RES) Write_WARNING(errors, u8"Se han ignorado decimales en %s. El valor guardado es %c%u %s. Se indicó %s.", key, bneg? '-':' ', val,uni,str);
	else Write_ERROR(errors, u8"Error desconocido al intentar convertir el valor de %s a una magnitud: %s.", key,str);
	if(file!=NULL) LOG_f(errors,NO_LINE,u8"    Clave leída en el archivo %s, línea %u\n",file,line);

done:
	if(err<0) errors->err_count++;
	else errors->war_count++;
}

void error_Float_err(int err, ErrorOpts *errors, const char8_t *file, uint line, const char8_t* key, const char8_t *str, float _unused(val), float max_val, float _unused(max_recom)){
	ifz(err) return;
	if(err<0){iferrwar(errors,LERR); else goto done;}
	else{iferrwar(errors,LERR); else goto done;}

	if(err==ERR_VAL_EMPTY) Write_ERROR(errors, u8"No se puede convertir el valor de %s a un float porque es una cadena vacía.",key);
	elif(err==ERR_VAL_NONUMBER) Write_ERROR(errors, u8"Al intentar convertir el valor de %s a float. Se ha indicado algo que no es un número: %s.", key,str);
	elif(err==ERR_VAL_NUMHUGE || err==ERR_VAL_HUGE) Write_ERROR(errors, u8"Al intentar convertir el valor de %s a float. El valor máximo posible es %f. Se ha indicado %s.", key,FLT_MAX,str);
	elif(err==ERR_VAL_RANGE) Write_ERROR(errors, u8"En la conversión del valor de %s a float. El valor máximo permitido es %f. Se ha indicado %s.", key,max_val,str);
	elif(err==WAR_VAL_RANGE) Write_WARNING(errors, u8"Valor anormalmente alto en la calve %s. Se encontró %s.", key,str);
	else Write_ERROR(errors, u8"Error desconocido al intentar convertir el valor de %s a un float: %s.", key,str);
	if(file!=NULL) LOG_f(errors,NO_LINE,u8"    Clave leída en el archivo %s, línea %u\n",file,line);

done:
	if(err<0) errors->err_count++;
	else errors->war_count++;
}

#undef GET_LINE
