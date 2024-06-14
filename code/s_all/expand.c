#define GET_LINE nlínea

#define is_expand(c) (c==KV_BeginExpand || c==KV_EndExpand)
#define is_cmdchar(c) (c==KV_BeginExpand || c==KV_EndExpand || c==KV_Substitute)
//Elimina todos los comandos que hubiera en s
static char8_t* make_string_obj(char8_t *s){
	while(*s!='\0' && !is_expand(*s)) s++;
	if(*s=='\0') return s;

	char8_t *pc=s;
	do{while(*s!='\0' && is_expand(*s)) s++;
		while(*s!='\0' && !is_expand(*s)) *pc++=*s++;
	}while(*s!='\0');
	*pc='\0';
	return pc;
}

//Elimina todos los comandos que hubiera en kv->c.str
sinline void make_string_macobj(const KeyVals *kvs, MacObj *kv){
	char8_t *pc=make_string_obj(KeyStrp(kv));
	kv->c.exp_pending=0;
	kv->c.strsize=(pdif)(pc-KeyStrp(kv))-1;
}

/* Devuelve un puntero a la posición del correspondiente paréntesis de cierre o bien:
	pc. Si pc no está apuntando a un paréntetsis de apertura
	un puntero al blanco, '\0' o caracter no permitido que detuvo la lectura.

    Si _cl no es NULL tiene que apuntar a un char8_t en el que la función guarda el
paréntesis de cierre correspondiente al paréntesis de apertura. Para saber si la función
ha encontrado con éxito el paréntesis de cierre ha de mirarse si *returned==*_cl.
Si ya sabemos cuál es el paréntesis de apertura se puede prescindir de _cl. Por ejemplo,
si *pc='(' al llamar a la función, esta habrá tenido éxito si tras la llamada *returned=')'.
*/
static char8_t* find_closing_paren_b0(const char8_t *pc, char8_t *_cl){
	u8int npar; //Level within parentheses
	char8_t op, cl;

	op=*pc;
	switch(*pc){
	case '[': cl=']'; break;
	case '(': cl=')'; break;
	case '{': cl='}'; break;
	case '<': cl='>'; break;
	default: assert("Shoud not get here");
		if(_cl!=NULL){if(op!='\0') *_cl='\0'; else *_cl=' ';}
		return (ICONST char8_t*)pc;
	}
	pc++;
	if(_cl!=NULL) *_cl=cl;

	npar=1;
	while(isnot_stn0(*pc)){
		if(*pc==op) npar++;
		elif(*pc==cl){ifz(--npar) break;}
		pc++;
	}
	return (ICONST char8_t*)pc;
}

/* Devuelve un puntero a la posición del correspondiente paréntesis de cierre o bien:
	pc. Si pc no está apuntando a un paréntetsis de apertura
	un puntero al '\n', '\0' o caracter no permitido que detuvo la lectura.

    Si _cl no es NULL tiene que apuntar a un char8_t en el que la función guarda el
paréntesis de cierre correspondiente al paréntesis de apertura. Para saber si la función
ha encontrado con éxito el paréntesis de cierre ha de mirarse, tras llamarla, si *pc==*_cl.
Si ya sabemos cuál es el paréntesis de apertura se puede prescindir de _cl. Por ejemplo,
si *pc='(' al llamar a la función, esta habrá tenido éxito si tras la llamada *pc=')'.
*/
static char8_t* find_closing_paren_n0(iconst char8_t *pc, char8_t *_cl){
	u8int npar; //Level within parentheses
	char8_t op, cl;

	op=*pc;
	switch(*pc){
		case '[': cl=']'; break;
		case '(': cl=')'; break;
		case '{': cl='}'; break;
		case '<': cl='>'; break;
		default: assert("Shoud not get here");
			if(_cl!=NULL){if(op!='\0') *_cl='\0'; else *_cl=' ';}
			return (ICONST char8_t*)pc;
	}
	pc++;
	if(_cl!=NULL) *_cl=cl;

	npar=1;
	while(*pc!='\0' && *pc!='\n'){
		if(*pc==op) npar++;
		elif(*pc==cl){ifz(--npar) break;}
		pc++;
	}
	return (ICONST char8_t*)pc;
}

/* Recibe un puntero, pc, que tiene que apuntar past el ( que abre la lista de argumentos.
Substituye cada coma que limita un argumento a partir de pc por un '\0'. Devuelve
un puntero al carácter que detuvo la lectura, que puede ser:
	) '\n' '\0'
Si nargs no es NULL guarda ahí el número de argumentos, que será uno más que el
número de '\0' escritos.
    Una coma limita un argumento si no está dentro de un par ().
*/
static char8_t* prepare_arg_list(char8_t *pc, uint8m *nargs){
	u8int npar; //Level within parentheses
	uint8m n; //Number of args

	npar=1; n=1; //We are already inside the first argument
	while(*pc!='\0' && *pc!='\n'){
		if(*pc=='(') npar++;
		elif(*pc==')'){ifz(--npar) break;}
		elif(*pc==',' && npar==1){*pc='\0'; ifunlike(++n==KV_NARGS_MAX) goto skip;}
		pc++;
	}
	goto salida;

skip:
	do{pc++;
		if(*pc=='(') npar++;
		elif(*pc==')'){ifz(--npar) break;}
	}while(*pc!='\0' && *pc!='\n');

salida:
	if(nargs!=NULL) *nargs=n;
	return pc;
}

/*La expansión siempre es total.
    Convierte a texto el valor de la kv, guardando el resultado en expanded y devolviendo
un puntero al '\0' del final de la cadena. Si la cadena aún incluye comandos estos se
se eliminan y se copiará como un string puro. En caso contrario, si aún no se
había parseado, lo hace y guarda el valor en value (si != string).*/
static char8_t* text___val_kv(const KeyVals *kvs, char8_t *expanded, MacObj *kv){
	if(kv->c.exp_pending){
		char8_t *pc=expanded, *s=KeyStrp(kv);
		do{while(*s!='\0' && is_cmdchar(*s)) s++;
			while(*s!='\0' && !is_cmdchar(*s)) *pc++=*s++;
		}while(*s!='\0');
		*--pc='\0';
		return pc;
	}
	//Parsearlo, si aún no había hecho
	if(kv->value.type==VAL_TYPE_unknown) parse_unknown(&kv->value,KeyStrp(kv));
	if(kv->value.type==VAL_TYPE_string){
		char8_t *p=strpcpy8(expanded,KeyStrp(kv)); *--p='\0';
		return p;
	}
	if(kv->value.type==VAL_TYPE_bool){
		ifz(kv->value.val){*expanded++='N'; *expanded++='o';}
		else{*expanded++='S'; *expanded++='i';}
		*expanded='\0'; return expanded;
	}

	//Entero, o entero con unidades
	if(kv->value.type==VAL_TYPE_uint || kv->value.type==VAL_TYPE_sMedida){
		uint n;
		if(kv->value.type==VAL_TYPE_sMedida && kv->value.sval<0) *expanded++='-', n=-kv->value.sval;
		else n=kv->value.val;
		expanded=str8___uint(expanded,n);
		if(kv->value.type!=VAL_TYPE_uint) expanded=strpcpy8(expanded,u8" dmm");
		return expanded;
	}

	//Float
	{
		float f;
		uint n;
		if(kv->value.fl<0) *expanded++='-', f=-kv->value.fl; else f=kv->value.fl;
		n=(uint)f;
		expanded=str8___uint(expanded,n);
		f-=n;
		*expanded++='.';
		if(f==0) *expanded++='0';
		else{
			dontimes(6,){
				f*=10.0f; n=(uint)f;
				*expanded++='0'+(char8_t)n;
				f-=n; if(f==0) break;
		}	}
		*expanded='\0';
		return expanded;
	}
}

/* Recibe pc, que apunta past el ) que cierra el nombre de la variable.
Avanza pc, primero past el ( que tiene que seguir y que comienza la lista de argumentos.
Ahí se deja *arg_list.
Luego avanza pc hasta el ), '\n' o '\0' que cierra la lista de argumentos.
En los dos últimos casos se establecerá err_code a VARS_RUNAWAY.
Si falta el ( de apertura se hará error->err=VAR_MISSING_ARGLIST, y *arg_list queda indeterminado.

Reemplaza cada coma de la lista de argumentos por '\0'.
En *nargs guarda el número de argumentos pasados, que es igual a 1+el nº de '\0' escritos.
Muestra mesajes de error y warning.

kv sólo se emplea para kv->nargs, para el mensaje de error. Si se pasa NULL se ignora.
*/
char8_t* consume_arg_list(char8_t *pc, char8_t **arg_list, uint8m *nargs, const char8_t *var_name, const MacFun *kv, ErrorOpts *errors, uint nlínea, const strLog *logs){
	while(is_st(*pc)) pc++;
	if(*pc!='('){
		if(logs->macros>=1){Error_s(errors,NO_LINE,u8"La variable es una función pero falta la lista de argumentos a contiuación. La expansión es vacía.");}
		else{ERROR(errors,u8"La variable %s es una función pero falta la lista de argumentos a contiuación. La expansión es vacía.",var_name);}
		errors->err_count++; errors->err=VAR_MISSING_ARGLIST;
		return pc;
	}

	*arg_list=++pc;
	pc=prepare_arg_list(pc,nargs);
	ifunlike(*pc!=')'){
		ERROR(errors,u8"No se encontró el paréntesis de cierre correspondiente al '(' de apertura: %SN",*arg_list-1);
		errors->err_count++; errors->err=VARS_RUNAWAY;
		return pc;
	}
	if(kv!=NULL && *nargs<kv->nargs){
		WARNING(errors,u8"La función %s se ha definido con %u argumentos, pero la lista () de argumentos que aparece tiene solo %u argumentos.",var_name,kv->nargs,*nargs);
		errors->war_count++;
	}
	elif(kv!=NULL && *nargs>kv->nargs){
		WARNING(errors,u8"La función %s se ha definido con %u argumentos, pero la lista () de argumentos que aparece tiene %u argumentos.",var_name,kv->nargs,*nargs);
		if(errors->err_war>=LWAR) LOG_s(errors,NO_LINE,INDENT_ERROR u8"Los argumentos restantes se ignoran\n");
		errors->war_count++;
	}

	return pc;
}

//End apunta al carácter de cierre, que no hace falta restaurar.
sinline void restaura_arg_list(char8_t *args, char8_t *end){
	while(args!=end){if(*args=='\0') *args=','; args++;}
}

/*En la cadena str reemplaza los KV_SUBSTITUE<n> por el argumento n-ésimo de args.
Los argumentos se empiezan a contar en 1. Si <n> es mayor que nargs, se reemplaza por la cadena vacía.
Tembién elimina todo KV_BeginExpand y KV_EndExpand que hubiera.
El resultado se guarda en pn, que tiene que tener espacio suficiente.
Devuelve un putnero al '\0' de cierre en pn.
*/
static char8_t* replace_args_clean_KVexpand(char8_t *pn, const char8_t *str, const char8_t *args, uint nargs);

/*Expande un texto que puede tener secciones KV_Begin/EndExpand. Se supone que el resto
ya está expandido. sto se da cuando se relee un texto que es el de reemplazo de una variable.
Copia la expansión en pn y devuelve pn avanzado.
pc: texto a expandir
btotal: false: expandir según las reglas
		true: expandir completamente.

Solamente se expanden las secciones encuadradas entre KV_BeginExpand y KV_EndExpand.
Si btotal es true, en lo copiado no habrá ningún comando, incluso en caso de error.
En caso de error, a partir del punto en que se produce el error copia la cadena eliminando
los comandos.
Return:
	Un puntero al '\0' del final de la cadena expandida en pn

 errors->err:
	0: Todo bien
	AT_NOMEM: Sin memoria
	>0: Algún error al expandir
*/
static char8_t* reexpand(char8_t *pn, char8_t *pc, KeyVals *kvs, bint btotal, ErrorOpts *errors, uint nlínea, const strLog *logs);

/*Expande la str de kv. Copia la expansión en pn y devuelve pn avanzado.
btotal: false: expandir según las reglas
		true: expandir completamente.

Si btotal es true, en lo copiado no habrá ningún comando, incluso en caso de error.
En caso de error, a partir del punto en que se produce el error copia la cadena eliminando
los comandos.
Return:
	Un puntero al '\0' del final de la cadena expandida en pn

 errors->err:
	0: Todo bien
	AT_NOMEM:	Sin memoria
	EXP_IMMEDIATE_RECURSIVE: Se llegó aquí expandiendo la propia variable kv.
	Otro >0:			Algún error al expandir.
*/
static char8_t* expand_obj(char8_t *pn, MacObj *kv, KeyVals *kvs, bint btotal, ErrorOpts *errors, uint nlínea, const strLog *logs);
static char8_t* expand_fun(char8_t *pn, MacFun *kv, const char8_t *arg_list, uint nargs, KeyVals *kvs, bint btotal, ErrorOpts *errors, uint nlínea, const strLog *logs);

/* Expande el texto entre el paréntesis de apertura apuntado por popen y el correspondiente
paréntesis de cierre. Expansión total. Guarda el resultado en expanded. popen no es
const porque puede modificarse provisionalmente, pero siempre se dejará como estaba.
	popen: b0-ended

Return:
    Un puntero TRAS el paréntesis de cierre que se corresponde con el paréntesis de apertura
al que apunta popen salvo que tras la llamada a la función sea:
	a) El puntero devuelto igual al popen pasado a la función.
o bien
	b) errors->err=VARS_RUNAWAY.

Si popen no apunta a un paréntesis de apertura devuelve inmediatamente popen y
errors->err=VARS_SYNTAX (el caso a) arriba). Si no se encuentra el paréntesis de cierre devuelve
popen hasta donde fue avanzando (por ejemplo, hasta un '\n') y errors->err=VARS_RUNAWAY
(el caso b) arriba).

errors->err:
	0: Todo bien
	AT_NOMEM:			Sin memoria
	VARS_RUNAWAY:		No se encontró el paréntesis de cierre.
	Otro >0:					Algún error al expandir.
*/
static char8_t* exptotal_bracketed_nombre(char8_t *popen, char8_t *expanded, KeyVals *kvs, ErrorOpts *errors, uint nlínea, const strLog *logs);

/*Expandir una cadena entre corchetes. '\0'-ended.
La sintaxis es [ ... ][ índice ] o [ ... ]{[ índice ]}. La sintaxis para índice es:

índice:
	n: Un número entero
	$<var>: La clave ha de ser (poder tiparse como) un número entero.
	$(var) o ${var}: La expansión de la variable ha de ser:
		índice

Expande [ ... ] según las reglas y del resultado selecciona la palabra índice-ésima,
descartando todas las demás. La primera palabra es la 1. Si índice es 0 o mayor que
el número de palabras presentes la expansión es vacía. En caso contrario:
	1. Si btotal, la palabra se expande total-mente.
	2a. Si btotal=0 y la forma era {[ índice ]}, esa palabra es el resultado final.
	2b. Si btotal=0 y la forma era [ índice ], la palabra se expande según las reglas.

El resultado queda en *_pn, '\0'-ended.
*_pn se avanza hasta ese '\0' .

Return: popen avanzado de la siguiente manera:
	A una posición past el ] o } de cierre, si todo está bien.
	Sin avanzar, si no se encontró el ] correspondiente al [ de inicio.
	Tras el ] correspondiente al [ inicial si no lo sigue un [ o {[.
	En el [ de apertura de [ ínicide ] si no se encuentra el correspondiente ].

errors->err:
	0: Todo bien
	AT_NOMEM:		Sin memoria
	VARS_RUNAWAY:	Runaway argument
	VAR_MUST_EXIST: Una variable $<var> encontrada en [ índice ] no existe.
	otro >0:		Otro error al parsear
*/
static char8_t* expand_Γ(char8_t *pc, char8_t **_pn, KeyVals *kvs, bint btotal, ErrorOpts *errors, uint nlínea, const strLog *logs);

/*Expande un texto completamente que no contiene KW_Begin/EndExpand. Guarda
el resultado en expanded y devolviendo un puntero al '\0' del final de la cadena expandida.
    text must be '\0'-terminated. No es const porque puede modificarse provisionalmente,
pero siempre se dejará como estaba.
    Aun en caso de error la función procesa todo el texto. O bien llega hasta el final
o bien lo descarta intencionadamente.
btotal: false: expandir según las reglas
		true: expandir completamente.

 Return:
	Un puntero al interior de expanded, al '\0' que cierra la cadena expandida.
 errors->err_code:
	0: Todo bien
	AT_NOMEM:	Sin memoria
	>0:				Se produjo algún error
*/
static char8_t* expand_text_0(char8_t *text, char8_t *expanded, KeyVals *kvs, bint btotal, ErrorOpts *errors, uint nlínea, const strLog *logs){
	char8_t *pn=expanded;
	char8_t *pc=text;
	int err_code=0;
	char8_t *arg_list; //Estos dos para
	uint8m nargs;	//macros que son funciones.

	while(*pc!='\0'){
		while(*pc!='\0' && *pc!='$') *pn++=*pc++;
		if(*pc=='\0') break;
		errors->err=0;
		pc++;
		if(*pc=='$'){*pn++='$'; pc++; continue;}
		if(*pc=='\0'){
			ERROR_s(errors,u8"Carácter $ al final de línea o de texto a expandir");
			err_code=VARS_SYNTAX; errors->err_count++;
			break;
		}
		if(*pc=='['){
			pc=expand_Γ(pc,&pn,kvs,btotal,errors,GET_LINE,logs);
			ifunlike(errors->err!=0){
				ifunlike(errors->err<0) goto salida;
				err_code=errors->err;
			}
			continue;
		}

		char8_t open=*pc, close; //Paréntesis de apertura: (', '{' o '<'
		KeyVal kv;
		char8_t var_name[VARNAME_MAX];
		if(!isopen(*pc) && !is_st(*pc)){ //Si is_st(*pc) dejaremos que el error salte más adelante
			open=Я8; //Si no hay, expansión normal.
			char8_t *ppio=pc;
			do pc++; while(*pc!='\0' && *pc!='(' && !is_st(*pc));
			if(*pc!='(') errors->err=VARS_RUNAWAY;
			else{*pc='\0'; strcpy8(var_name,ppio); *pc='(';}
		}else{
			pc=exptotal_bracketed_nombre(pc,var_name,kvs,errors,GET_LINE,logs);
			close=pc[-1]; //Para un eventual mensaje de error
		}
		ifunlike(errors->err!=0){ //Saltarse la variable. pc ya apunta past ella, si err!=VARS_RUNAWAY
			ifunlike(errors->err<0) goto salida;
			//Aquí no mostramos el texto porque puede faltar el principio de "texto". En reexpand se llama
			//a esta función a partir del primer $.
			err_code=errors->err;
			if(errors->err==VARS_RUNAWAY && is_st(*pc)) pc++;
			continue;
		}

		if(logs->macros>=1) LOG_sss(errors,GET_LINE,u8"Expandiendo la variable ",var_name,u8" ...\n");
		kv=find_key_0(kvs,var_name);
		if(kv.p.obj==NULL){ //la cadena $(<var>) desaparece
			if(logs->macros>=1) LOG_s(errors,NO_LINE,u8"\tLa variable no existe. La expansión es vacía.\n");
			continue;
		}
		if(open!='<' && logs->macros>=1) LOG_ssnl(errors,NO_LINE,u8"\texpansión: ",KeyStrp(kv.p.obj));

		if(kv.macro_type==MACRO_FUNC || open==Я8){
			const MacFun *pf= kv.macro_type==MACRO_FUNC ? kv.p.fun : NULL;
			pc=consume_arg_list(pc,&arg_list,&nargs,var_name,pf,errors,nlínea,logs);
			if(errors->err!=0){
				err_code=errors->err;
				if(errors->err==VAR_MISSING_ARGLIST || errors->err==VARS_RUNAWAY) continue;
				errors->err=0;
			}
			*pc++='\0'; //Dejar pc past el ')'
			if(kv.macro_type!=MACRO_FUNC){
				ERROR(errors,u8"No se encuentra la marco %s entre las funciones. Si el carácter '$' va seguido de un nombre sin paréntesis, en lugar de $(nombre), tiene que ser una función.", var_name);
				errors->err_count++; err_code=VAR_MUSTBE_FUNC;
				continue;
			}
			if(open==Я8) open='(';
			elif(open!='(' && open!='{'){
				ERROR(errors,u8"Se encontró $%c%s%c. La macro %s es una función y su invocación ha de ser ${%s} o $(%s). Se tratará como si hubiera sido esto último.", open,var_name,close,var_name,var_name,var_name);
				errors->err_count++; err_code=VARS_SYNTAX;
				open='(';
			}
		}
		if(btotal && open=='{') open='(';
		if(open=='('){
			if(kv.macro_type==MACRO_SPECIAL) ;
			elif(kv.macro_type==MACRO_OBJECT) pn=expand_obj(pn,kv.p.obj,kvs,btotal,errors,GET_LINE,logs);
			else{pn=expand_fun(pn,kv.p.fun,arg_list,nargs,kvs,btotal,errors,GET_LINE,logs);
				restaura_arg_list(arg_list,pc);
			}
			ifunlike(errors->err!=0){
				ifunlike(errors->err<0) goto salida;
				err_code=errors->err;
				if(err_code==EXP_IMMEDIATE_RECURSIVE){
					err_code=EXPANSION_RECURSIVE;
					*pn++='$'; *pn++='(';
					pn=strpcpy8(pn,var_name); *pn++=')';
					if(kv.macro_type==MACRO_FUNC){
						*pn++='('; while(arg_list!=pc) *pn++=*arg_list++;
					}
			}	}
		}elif(open=='{'){ //Expand once
			if(kv.macro_type==MACRO_SPECIAL) ;
			elif(kv.macro_type==MACRO_OBJECT) pn=strpcpy8(pn,KeyStrp(kv.p.obj))-1; //El espacio final sobrante
			else{
				*pn++=KV_BeginExpand;
				pn=replace_args_clean_KVexpand(pn,KeyStrp(kv.p.fun),arg_list,nargs);
				pn[-1]=KV_EndExpand; //En el ugar del ' ' sobrante.
				restaura_arg_list(arg_list,pc);
			}
		}else{ //'<' Expand the value. For strings, this means copy the string (no further expansion)
			pn=text___val_kv(kvs,pn,kv.p.obj);
			if(logs->macros>=1) LOG_ssnl(errors,NO_LINE,u8"\texpansión: ",pn);
		}
	}

	errors->err=err_code;
salida:
	*pn='\0';
	return pn;
}

/*Expande de un texto según las reglas (exptext_reglas_0) o total (exptext_total_0).
Guarda el resultado en expanded. text no es const porque puede modificarse provisionalmente,
pero siempre se dejará como estaba.
	text: '\0'-terminated.

 Return:
	Un puntero al interior de expanded, al '\0' que cierra la cadena expandida.
 errors->err_code:
	0: Todo bien
	AT_NOMEM: Sin memoria
	>0: Algún error en la expansión
*/
sinline char8_t* exptext_reglas_0(char8_t *text, char8_t *expanded, KeyVals *kvs, ErrorOpts *errors, uint nlínea, const strLog *logs){
	return expand_text_0(text,expanded,kvs,false,errors,nlínea,logs);
}
sinline char8_t* exptext_total_0(char8_t *text, char8_t *expanded, KeyVals *kvs, ErrorOpts *errors, uint nlínea, const strLog *logs){
	return expand_text_0(text,expanded,kvs,true,errors,nlínea,logs);
}
/*line is '\n'-terminated. Expande de manera recursiva, según las reglas, guardando el resultado
en expanded. line puede modificarse provisionalmente, pero siempre se dejará como estaba.
 Return:
	Puntero al '\0' de final de expanded
 errors->err_code:
	0: Todo bien
	AT_NOMEM: Sin memoria
	>0: Algún error en la expansión
*/
static char8_t* exptext_reglas_n(char8_t *line, char8_t expanded[MAX_LINE_EX], KeyVals *kvs, ErrorOpts *errors, uint nlínea, const strLog *logs){
	char8_t *pend, *pn;
	for(pend=line;*pend!='\n';pend++);
	*pend='\0'; pn=exptext_reglas_0(line,expanded,kvs,errors,nlínea,logs); *pend='\n';
	return pn;
}

static char8_t* exptotal_bracketed_nombre(char8_t *popen, char8_t *expanded, KeyVals *kvs, ErrorOpts *errors, uint nlínea, const strLog *logs){
	ifunlike(!isopenvar(*popen)){errors->err=VARS_SYNTAX; errors->err_count++; return popen;}

	char8_t cl;
	char8_t *pvar=popen+1;
	popen=find_closing_paren_b0(popen, &cl);
	if(*popen!=cl){
		--pvar;
		ERROR(errors,u8"No se encontró el paréntesis de cierre correspondiente al '%c' de apertura: %SB", *pvar,pvar);
		errors->err_count++;
		errors->err=VARS_RUNAWAY; return popen;
	}
	*popen='\0';
	exptext_total_0(pvar,expanded,kvs,errors,GET_LINE,logs);
	//Devolver VARS_RUNAWAY solamente si no se encuentra el paréntesis de cierre por el que se ha llamado a esta función
	if(errors->err==VARS_RUNAWAY) errors->err=VARS_SYNTAX;
	*popen=cl;
	return popen+1;
}

static char8_t* reexpand(char8_t *pn, char8_t *pc, KeyVals *kvs, bint btotal, ErrorOpts *errors, uint nlínea, const strLog *logs){
	int err_code=0;
	char8_t * const text=pc;

	while(*pc!='\0'){
		if(*pc!=KV_BeginExpand){*pn++=*pc++; continue;}
		char8_t *pend=pc; do pend++; while(*pend!=KV_EndExpand); // && *pend!='\0');
		*pend='\0';
		pn=expand_text_0(pc+1,pn,kvs,btotal,errors,GET_LINE,logs);
		ifunlike(errors->err!=0){
			err_code=errors->err;
			ifunlike(err_code<0) break;
			wrong_text_0(errors,text);
		}
		*pend=KV_EndExpand;
		pc=pend+1;
	}

	errors->err=err_code;
	return pn;
}

//com: To keep track of recursive expansion and to display the name.
//str: The string to expand. com's str or a preprocessed version thereof.
//Sólo expande los segmentos entre KV_BeginExpand y KV_EndExpand. El resto se supone que ya está expandido
static char8_t* expand_com_str(char8_t *pn, MacroCommon *com, char8_t *str, KeyVals *kvs, bint btotal, ErrorOpts *errors, uint nlínea, const strLog *logs){
	if(com->expding){
		ERROR(errors,u8"Expansión recursiva de la variable %s",kvs->strs.ppio+com->key);
		*pn='\0'; errors->err=EXP_IMMEDIATE_RECURSIVE;
		errors->err_count++;
		return pn;
	}

	if(logs->macros>=2) LOG_ssnl(errors,GET_LINE,u8"Antes de expandir: ",str);
	com->expding=1;
	pn=reexpand(pn,str,kvs,btotal,errors,nlínea,logs);
	com->expding=0;
	*--pn='\0'; // A kw's value ends in ' ''\0'. The ' ' is not part of the value.
	return pn;
}

sinline char8_t* expand_obj(char8_t *pn, MacObj *kv, KeyVals *kvs, bint btotal, ErrorOpts *errors, uint nlínea, const strLog *logs){
	if(!kv->c.exp_pending) pn=strpcpy8(pn,KeyStrp(kv))-1;
	else pn=expand_com_str(pn,&kv->c,KeyStrp(kv),kvs,btotal,errors,nlínea,logs);
	return pn;
}

//En la cadena str reemplaza los KV_SUBSTITUE<n> por el argumento n-ésimo de args.
//Los argumentos se empiezan a contar en 1. Si <n> es mayor que nargs, se reemplaza por la cadena vacía.
//Tembién elimina todo KV_BeginExpand y KV_EndExpand que hubiera.
//El resultado se guarda en pn, que tiene que tener espacio suficiente.
//Devuelve un putnero al '\0' de cierre en pn.
static char8_t* replace_args_clean_KVexpand(char8_t *pn, const char8_t *str, const char8_t *args, uint nargs){
	while(*str!='\0'){
		if(is_expand(*str)){str++; continue;}
		if(*str!=KV_Substitute){*pn++=*str++; continue;}
		str++;
		uint8m n=*str++-'1';
		if(n>=nargs) continue;
		const char8_t *arg=args;
		while(n){n--;
			while(*arg!='\0') arg++; arg++;
		}
		while(is_st(*arg)) arg++;
		if(*arg=='\0') continue; //Argumento vacío
		const char8_t *end=arg; do end++; while(*end!='\0');
		do end--; while(is_st(*end)); end++;
		while(arg!=end) *pn++=*arg++;
	}
	*pn='\0';
	return pn;
}

//Se expande en pn. Devuelve un puntero al final de esa expansión.
//Tras la llamada puede ser error->err<0 (AT_NOMEM), así como otros >=0.
static char8_t* expand_fun(char8_t *pn, MacFun *kv, const char8_t *arg_list, uint nargs, KeyVals *kvs, bint btotal, ErrorOpts *errors, uint nlínea, const strLog *logs){
	//if(!kv->c.exp_pending) ... No sirve porque entre los argumentos puede haber texto que reemplazar

	char8_t *exp2; //Maybe change to VLA
	{uint n=0; //Obtener la longitud máxima de los argumentos de arg_list
	const char8_t *p2=arg_list;
	dontimes(nargs,){
		const char8_t *p1=p2;
		while(*p2!='\0') p2++;
		uint k=(pdif)(p2-p1);
		maxeq(n,k);
		p2++;
	}
	n=kv->c.strsize+n*kv->nreplacements+2+2; //Un +2 es necesrio si n=0. El otro, por
	exp2=n_malloc(char8_t,n);
	}
	ifunlike(exp2==NULL){errors->err=AT_NOMEM; return pn;}
	*exp2=KV_BeginExpand;
	{char8_t *pc=replace_args_clean_KVexpand(exp2+1,KeyStrp(kv),arg_list,nargs);
	pc[-1]=KV_EndExpand; *pc++=' '; *pc='\0';}
	pn=expand_com_str(pn,&kv->c,exp2,kvs,btotal,errors,nlínea,logs);

	free(exp2);
	return pn;
}

//No se permiten '\n' por en medio de la construcción, así que es como si pc estuviera '\n'- o '\0'-ended
static char8_t* expand_Γ(char8_t *pc, char8_t **_pn, KeyVals *kvs, bint btotal, ErrorOpts *errors, uint nlínea, const strLog *logs){
	char8_t *pn=*_pn, *pclose;
	int err_code=0; //Guarda el de las expansiones de los [ ... ]
	*pn='\0';
	//pn no se avanzará hasta que no se haya extraído la palabra value-ésima
	char8_t expanded2[MAX_LINE_EX];

	assert(*pc=='[');
	char8_t *pend=find_closing_paren_n0(pc,NULL);
	ifunlike(*pend!=']'){
		char8_t c=*pend; *pend='\0';
		ERROR(errors,u8"No se encontró el paréntesis de cierre correspondiente al '[' de apertura: %s",pc);
		*pend=c;
		errors->err_count++; errors->err=VARS_RUNAWAY;
		return pend;
	}
	{char8_t c=*pend; *pend='\0';
	pclose=exptext_reglas_0(pc+1,pn,kvs,errors,GET_LINE,logs); *pend=c;}
	pend++;
	//A partir de aquí para salir por error hay que IR A SALIDA, dejando pc en su posición final
	ifunlike(errors->err!=0){
		ifunlike(errors->err<0){pc=pend; goto salida;}
		char8_t c=*pend; *pend='\0'; ChkErrLiteral(errors,INDENT_ERROR u8"Al expandir la cadena %s\n",pc);
		*pend=c;
		err_code=errors->err;
	}
	pc=pend;
	errors->err=0;
	//La expansión de [] ha quedado en pn. pclose apunta al '\0' que la cierra

	bint braced=0;
	str_no_st(pc);
	if(*pc=='{') braced=1, pc++;
	if(*pc!='['){
		if(*pc=='\n' || *pc=='\0'){ERROR_s(errors,u8"Tras el $[ ... ] falta el [...] o {[...]} que ha de seguir");}
		else{ERROR(errors,u8"Un $[ ... ] ha de ir seguido de [...] o {[...]}. Se encontró: %c",*pc);}
		if(*pc=='\n' || *pc=='\0') errors->err=VARS_RUNAWAY;
		else{if(braced) pc--; errors->err=VARS_SYNTAX;}
		errors->err_count++;
		goto salida;
	}
	pend=find_closing_paren_n0(pc,NULL);
	ifunlike(*pend!=']'){
		char8_t c=*pend; *pend='\0';
		ERROR(errors,u8"No se encontró el paréntesis de cierre correspondiente al '[' de apertura: %s",pc);
		*pend=c;
		errors->err_count++; errors->err=VARS_RUNAWAY;
		pc=pend; goto salida;
	}
	if(braced && pend[1]!='}'){
		++pend; str_no_st(pend);
		const char8_t *s, *ellipsis;
		if(*pend=='}') s=u8"En un {[...]} el '}' de cierre tiene que venir inmediatamente a continuación del ']': %s%s";
		else s=u8"Tras el ']' de cierre falta el '}': %s%s";
		char8_t c=*pend; if(c!='\0' && c!='\n') pend++;
		if(*pend=='\0' || *pend=='\n') ellipsis=pend; else ellipsis=u8"...";
		*pend='\0'; ERROR_f(errors,s,pc-1,ellipsis);
		if(c!='\0' && c!='\n') pend--;
		*pend=c;
		errors->err_count++; errors->err=VARS_SYNTAX;
		pc=pend; goto salida;
	}
	{char8_t c=*pend; *pend='\0'; exptext_total_0(pc+1,expanded2,kvs,errors, GET_LINE,logs); *pend=c;}
	pend++;
	ifunlike(errors->err!=0){
		ifunlike(errors->err<0){pc=pend+(braced!=0); goto salida;}
		char8_t c=*pend; *pend='\0'; ChkErrLiteral(errors,INDENT_ERROR u8"Al expandir la cadena %s\n",pc);
		*pend=c;
		err_code=errors->err;
	}
	pc=pend+(braced!=0);
	//pc ya está apuntado a su posición final

	u16int value=0;
	bint bneg=0;
	{char8_t *pval=expanded2;
	str_no_st(pval);
	if(*pval=='\0'){
		ERROR_s(errors,u8"El contenido de un [ ] que tiene que indicar el número de palabra está vacío");
		errors->err_count++; errors->err=VARS_SYNTAX;
		goto salida;
	}
	if(*pval=='-') bneg=1, pval++;
	while((char8_t)(*pval-'0')<=9){
		value*=10, value+=(*pval-'0'); pval++;
		ifunlike(value>=BRACKETS_MAX_N) break;
	}
	while((char8_t)(*pval-'0')<=9) pval++;
	str_no_st(pval);
	ifunlike(*pval!='\0'){
		value=0;
		errors->err=VARS_SEMANTICS; errors->err_count++;
		pval=expanded2; str_no_st(pval);
		ERROR_s(errors,u8"El contenido de un [ ] que indica el número de palabra ha de ser un número entero");
		ChkErrLiteral(errors,INDENT_ERROR u8"Se encontró: %s\n",pval);
		goto salida;
	}
	ifunlike(value>=BRACKETS_MAX_N){
		value=0;
		errors->err=VARS_SEMANTICS; errors->err_count++;
		pval=expanded2; str_no_st(pval);
		ERROR(errors,u8"El número de palabra indicado dentro de [ ] es demasiado grande: %s",pval);
		goto salida;
	}}

	//La expansión de [] ha quedado en pn. pclose apunta al '\0' que la cierra. Hay que extraer
	//la palabra value-ésima. Dejaremos la palabra seleccionada copiada en pn_begin, '\0'-ended.
	//Hay que dejarla ahí, porque es donde la espera el que llama a esta función.
	char8_t *pn_begin=pn;
	if(value==0 || *pn=='\0') goto salida_value0;

	if(!bneg){
		bint in_expand=0;
		if(value==1 && isnot_stn(*pn)){
			while(isnot_stn0(*pn)){
				if(*pn==KV_BeginExpand) in_expand=1;
				elif(*pn==KV_EndExpand) in_expand=0;
				pn++;
			}
		}else{
			value--;
			str_no_stn(pn);
			while(value!=0 && *pn!='\0'){ value--;
				while(isnot_stn0(*pn)){
					if(*pn==KV_BeginExpand) in_expand=1;
					elif(*pn==KV_EndExpand) in_expand=0;
					pn++;
				}
				if(*pn!='\0') str_no_stn(pn);
			}
			if(*pn=='\0') goto salida_value0;

			char8_t *p=pn_begin;
			if(in_expand) *p++=KV_BeginExpand;
			while(isnot_stn0(*pn)){
				if(*pn==KV_BeginExpand) in_expand=1;
				elif(*pn==KV_EndExpand) in_expand=0;
				*p++=*pn++;
			}
			pn=p;
		}
		if(in_expand) *pn++=KV_EndExpand;
	}else{
		bint in_expand=0;
		u16int c=0; //Recordar cuántas posiciones avanzamos pn_begin
		{char8_t *p=pn_begin; str_no_stn(p); c=(pdif)(p-pn_begin);
		if(*p=='\0') goto salida_value0;}
		pn_begin+=c; //pn_begin apunta a un comienzo de palabra.

		pn=pclose-1;
		//Al princpio de cada iteración pn apunta al último _ de xxxxx__
		//En la primera iteración puede no existir ningún _ y pn apuntaría a la última x.
		if(pn==pn_begin) value--; //Por eso esta excepción.
		while(value!=0 && pn>pn_begin){ value--;
			while(is_stn(*pn)) pn--;
			while(pn!=pn_begin && isnot_stn(*pn)){
				if(*pn==KV_EndExpand) in_expand=1;
				elif(*pn==KV_BeginExpand) in_expand=0;
				pn--;
			}
		}
		if(pn!=pn_begin) pn++; //pn estaba atrasado una posición
		pn_begin-=c;
		if(value!=0) goto salida_value0;

		if(pn==pn_begin){ //hence, era c=0
			while(isnot_stn0(*pn)){
				if(*pn==KV_BeginExpand) in_expand=1;
				elif(*pn==KV_EndExpand) in_expand=0;
				pn++;
			}
		}else{
			char8_t *p=pn_begin;
			if(in_expand) *p++=KV_BeginExpand;
			while(isnot_stn0(*pn)){
				if(*pn==KV_BeginExpand) in_expand=1;
				elif(*pn==KV_EndExpand) in_expand=0;
				*p++=*pn++;
			}
			pn=p;
		}
		if(in_expand) *pn++=KV_EndExpand;
	}
	*pn='\0'; //pn cierra la palabra seleccionada
	//Fin seleccionar la palabra value-ésima. Ha quedado en pn_begin, '\0'-ended.

	if(btotal) braced=0;
	if(!braced){//hay que expandir
		while(*pn_begin!='\0' && *pn_begin!=KV_BeginExpand) pn_begin++; //Mirar si no hay nada que expandir
		if(*pn_begin==KV_BeginExpand){
			char8_t exp2[40];
			char8_t *pprov;
			uint size=(pdif)(pn-pn_begin)+1;
			if(size<40) pprov=&exp2[0];
			else{
				pprov=n_malloc(char8_t,size);
				ifunlike(pprov==NULL){errors->err=AT_NOMEM; goto salida;}
			}
			strcpy8(pprov,pn_begin);
			pn=reexpand(pn_begin,pprov,kvs,btotal,errors,GET_LINE,logs);
			//("Al expandir la palabra nº %u de [pn_antes_de_machacarlo]", value)
			if(pprov!=&exp2[0]) free(pprov);
		}
	}

salida_value0:
	pn=pn_begin;
	if(errors->err==0) errors->err=err_code;
salida:
	*pn='\0'; *_pn=pn;
	return pc;
}

/* Reemplaza el 'value' de kv por el resultado de su expansión completa.
Siempre y en cualquier caso, en kv queda un string puro; e.d., sin comandos, salvo
si se devuelve AT_NOMEM.
    Devuelve el valor guardado en errors->err:
	0: Todo bien
	AT_NOMEM.
	>0: Algún error al expandir
*/
static int fullexpand_kv(KeyVals *kvs, MacObj *kv, ErrorOpts *errors, uint nlínea, const strLog *logs){
	char8_t expanded[MAX_LINE_EX];
	if(!kv->c.exp_pending) return 0;

	expand_com_str(expanded,&kv->c,KeyStrp(kv),kvs,true,errors,nlínea,logs);
	uint newstr=(pdif)(kvs->strs.next-kvs->strs.ppio);
	GC8_adds(kvs->strs,expanded, return errors->err=AT_NOMEM);
	kv->c.str=newstr;
	kv->c.strsize=(pdif)(kvs->strs.next-KeyStrp(kv));
	*kvs->strs.next++=' '; Gadd(kvs->strs,char8_t,'\0', return errors->err=AT_NOMEM);
	kv->c.exp_pending=0;

	return errors->err;
}

//Además de escribir el código actualiza atend_nfile
//Return: 0, AT_NOMEM
sinline int insert_file_atend(Globals *globals){
	GC8_adds(globals->atend,u8"FILE ", return AT_NOMEM);
	GC8_adds(globals->atend,globals->kvs.strs.ppio+globals->kvs.filenames.ppio[globals->state.nfile], return AT_NOMEM);
	Gadd(globals->atend,char8_t,'\n', return AT_NOMEM);
	globals->state.atend_nfile=globals->state.nfile;
	return 0;
}

//Return: 0 , AT_NOMEM
static int insert_line_atend(Globals *globals, uint lc, char8_t *expanded){
	char8_t *pe=strpcpy8(expanded,u8"LINE= ");
	pe=str8___uint(pe,lc); *pe++='\n';
	uint n=(pdif)(pe-expanded); //El último carácter copiado será el '\n'
	pe=expanded;
	Gadd_n(globals->atend,char8_t,pe,n,return AT_NOMEM);
	return 0;
}

#undef GET_LINE
