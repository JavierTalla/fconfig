#define GET_LINE nlínea

/* Parsea una cadena de texto para su comparación (o para otro propósito). '\n'-ended.

mem: El miembro del if a parsear. Terminado por '\n'. Puede modificarse provisionalmente,
		pero siempre se dejará como estaba.
blógico: Si se permite un parseado lógico, en caso de que cumpla la sintaxis.
		Si es true, se asignará 1 si cumple el formato de un if lógico, 0 si no.
val: Dónde guardar el valor parseado. El campo val solamente tiene significado
		si es de un tipo no string.
expanded: Donde se guarda la cadena expandida.
globals: Eso.

Return:
	0: Todo bien
	PARSEIF_Syntax: Un error de syntaxis (alguna variable mal indicada)
	PARSEIF_VarUndefined: La variable de la expansión lógica no está definida

En caso de error _no_ modifica err_count (el que llama decidirá si es un error, warning o nada)
Los caracteres-comando que queden tras la expansión se tratan como otros cualesquiera.
*/
static int parse_if_member(char8_t *mem, bint *blógico, ObjectValue *val, char8_t expanded[MAX_LINE_EX], KeyVals *kvs, ErrorOpts *errors, uint nlínea, const strLog *logs){
	char8_t c, *pend; //Para apuntar al fin de la clave en un if lógico
	expanded[0]='\0';

	if(*blógico){
	   *blógico=0;
	   if(mem[0]=='$' && mem[1]=='<'){
		char8_t *pc;
		pc=find_closing_paren_b0(mem+1,NULL);
		if(*pc!='>'){
			--mem;
			ERROR(errors,u8"No se encontró el paréntesis de cierre correspondiente al '%c' de apertura: %SB", *mem,mem);
			errors->err=VARS_RUNAWAY; errors->err_count++;
			return errors->err;
		}
		pend=pc++;
		while(is_st(*pc)) pc++;
		if(*pc=='\n') *blógico=1;
	} }

	if(!*blógico){
		exptext_reglas_n(mem,expanded,kvs,errors,GET_LINE,logs);
		if(errors->err!=0) return errors->err;
		parse_unknown(val,expanded);
	}else{
		KeyVal kv;
		c=*pend; *pend='\0';
		kv=find_key_0(kvs,mem+2);
		*pend=c;
		if(kv.p.obj==NULL){
			errors->err=PARSEIF_VarUndefined; errors->err_count++;
			return errors->err;
		}
		text___val_kv(kvs,expanded,kv.p.obj);
		*val=kv.p.obj->value;
	}

	errors->err=0;
	return 0;
}

/*Compara las strings teniendo en cuenta correctamente los caracteres de control,
que pueden estar en posiciones distintas en una y otra string y aun así ser idénticas
de significado.
Return:	0 si son iguales
			1 si son distintas
*/
static bint cmp_kvstrings(const char8_t *s1, const char8_t *s2){
	bint b1, b2;

	//There can be KV_EndExpand immediately followed by KV_BeginExpand, but
	//the opposite is not possible. Do not rely on that.
	b2=b1=0;
	while(*s1!='\0'){
		if(*s1==KV_EndExpand) s1++, b1=0;
		if(*s1==KV_BeginExpand) s1++, b1=1;
		if(*s2==KV_EndExpand) s2++, b2=0;
		if(*s2==KV_BeginExpand) s2++, b2=1;
		ifunlike(*s1==KV_EndExpand) continue;
		ifunlike(*s2==KV_EndExpand) continue;

		if(*s1!=*s2) return 1;
		if(*s1=='$' && b1!=b2) return 1;
		s1++, s2++;
	}
	while(*s2==KV_BeginExpand || *s2==KV_EndExpand) s2++;
	return *s2!='\0';
}

#undef GET_LINE
