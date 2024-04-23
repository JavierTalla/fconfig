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

//Llama a check_Entero_range y error_Entero
sinline void check_Entero(ErrorOpts *errors, const char8_t *file, uint line, const char8_t* key, const char8_t *str, uint val, uint max_val, uint max_recom){
	check_Entero_range(&errors->err,val,max_val,max_recom);
	error_Entero(errors->err,errors,file,line,key,str,val,max_val,max_recom);
}

//Llama a check_uMagnitud_range y error_Magnitud
sinline void check_Magnitud(ErrorOpts *errors, const char8_t *file, uint line, const char8_t* key, const char8_t *str, uint val, uint max_val, uint max_recom, const char8_t *uni){
	check_uMagnitud_range(&errors->err,val,max_val,max_recom);
	error_Magnitud(errors->err,errors,file,line,key,str,val,max_val,max_recom,uni);
}

//Llama a check_sMagnitud_range y error_Magnitud
sinline void check_sMagnitud(ErrorOpts *errors, const char8_t *file, uint line, const char8_t* key, const char8_t *str, ssint sval, uint max_val, uint max_recom, const char8_t *uni){
	check_sMagnitud_range(&errors->err,sval,max_val,max_recom);
	error_Magnitud(errors->err,errors,file,line,key,str,sval,max_val,max_recom,uni);
}

//Llama a check_uFloat_range y error_Float
sinline void check_uFloat(ErrorOpts *errors, const char8_t *file, uint line, const char8_t* key, const char8_t *str, float val, float max_val, float max_recom){
	check_uFloat_range(&errors->err,val,max_val,max_recom);
	error_Float(errors->err,errors,file,line,key,str,val,max_val,max_recom);
}

//Llama a check_sFloat_range y error_Float
sinline void check_sFloat(ErrorOpts *errors, const char8_t *file, uint line, const char8_t* key, const char8_t *str, float val, float max_val, float max_recom){
	check_sFloat_range(&errors->err,val,max_val,max_recom);
	error_Float(errors->err,errors,file,line,key,str,val,max_val,max_recom);
}

//Devuelve el error de lectura. (0, si no hubo)
int parse_Bool(ObjectValue *dato, const char8_t *pc){
	BoolValidado d;

	d=lee_valida_Bool(&pc);
	if(d.err>=0) dato->b=d.val, dato->type=VAL_TYPE_bool;
	else dato->type=VTYPE_erroneous_bool;
	return d.err;
}

//Devuelve el error de lectura. (0, si no hubo)
int parse_Entero(ObjectValue *dato, const char8_t *pc){
	EnteroValidado d;

	d=lee_valida_Entero(&pc,Я,Я);
	if(d.err>=0) dato->val=d.val, dato->type=VAL_TYPE_uint;
	else dato->type=VTYPE_erroneous_uint;
	return d.err;
}

//Devuelve el error de lectura. (0, si no hubo)
int parse_Medida(ObjectValue *dato, const char8_t *pc){
	bint bneg;
	sMagnitudValidada d;

	ifnz(bneg=*pc=='-') pc++;
	d=lee_valida_sLongitudTabla(&pc,MAX_MEDIDA_LARGA,MAX_MEDIDA_LARGA,UNI_DMM);
	if(d.err>=0){ //Hence hay unidades y d.val<=MAX_MEDIDA_LARGA
		dato->sval=d.val;
		if(bneg) dato->sval=-dato->sval;
		dato->type=VAL_TYPE_sMedida;
	}else{
		dato->type=VTYPE_erroneous_sMedida;
	}
	return d.err;
}
//Devuelve el error de lectura. (0, si no hubo)
int parse_Float(ObjectValue *dato, const char8_t *pc){
	FloatValidado d;

	d=lee_valida_sFloat(&pc,FLT_MAX,FLT_MAX);
	if(d.err>=0) dato->fl=d.val, dato->type=VAL_TYPE_float;
	else dato->type=VTYPE_erroneous_float;
	return d.err;
}


/*Lee una cadena de texto que so sabemos qué es. Si la función determina que puede ser un booleano,
una longitud-tabla o un entero, lo guarda. Estre estas dos últimas tiene preferenica la
longitud-tabla. Si no, será un string y el contenido de dato tras la llamada es indefinido.
    En cualquier caso asigna dato->type.
    Devuelve el error de lectura. (0, si no hubo)

The parsed value need not exhaust the string; i. e., more data may follow. A string
matching the syntax of a boolean value will yield a boolean. Otherwise, if it is an integer,
pos. or neg., a valid unit follows and the measure has absolut value <= MAX_MEDIDA_LARGA
when expressed in dmm, it is parsed as a signed measure. Otherwise, if it is a number
then, if it contains '.' it is a float; if it does not and is not negative it is an integer.
If nothing of the previous holds, it is a string.
*/
int parse_unknown(ObjectValue *dato, const char8_t *pc){
	bint bneg, bentero;
	MagnitudValidada d;

	if(is_stn0(*pc)){
		dato->type=VAL_TYPE_string;
		return ERR_VAL_EMPTY;
	}

	d.err=parse_Bool(dato,pc);
	if(d.err>=0) return d.err;

	ifnz(bneg=*pc=='-') pc++;
	if(*pc<'0' || *pc>'9'){
		dato->type=VAL_TYPE_string;
		return 0;
	}

	const char8_t *p;
	p=pc; do p++; while(*p>='0' && *p<='9');
	bentero=!bneg && is_stn0(*p);

	{const char8_t *pc2=pc;
	d=lee_valida_LongitudTabla(&pc2,MAX_MEDIDA_LARGA,MAX_MEDIDA_LARGA,UNI_DMM);}
	dato->val=d.val;
	if(d.err>=0){ //Hence hay unidades y d.val<=MAX_MEDIDA_LARGA
		if(bneg) dato->sval=(ssint)-dato->val;
		dato->type=VAL_TYPE_sMedida;
		return d.err;
	}

	if(*p=='.'){ //float
		if(bneg) pc--;
		d.err=parse_Float(dato,pc);
		if(d.err<0){dato->type=VAL_TYPE_string; d.err=0;}
		return d.err;
	}
	//Here d.err<0
	if(bentero){
		d.err=parse_Entero(dato,pc); //Mirar que no es huge (aunque de momento no se comprueba)
		dato->type=VAL_TYPE_uint;
	}
	if(d.err<0){
		dato->type=VAL_TYPE_string;
		d.err=0;
	}
	return d.err;
}
