//Comprueba que la combinación de kw y ass es válida
static bint kwass_isvalid(KwAss kwass){
	if(KeyWords[kwass.kw].flags&KWFLAG_Assignment){
		return (kwass.ass==ASS_NONE) != ((KeyWords[kwass.kw].flags&KWFLAG_Texto)==0);
	}
	if(kwass.kw>=KeyWord_Min && kwass.ass==ASS_EqDef) return false;
	return true;
}

static Tok_Assign ass_classify(const char8_t *pc){
	if(*pc=='='){
		if(pc[1]=='=') return ASS_NONE;
		return ASS_Eq;
	}
	if(*pc==':' && pc[1]=='=') return ASS_EqDef;
	return ASS_NONE;
}

sinline Tok_Relation rel_classify(char16_t op){
	Tok_Relation i=1;
	for(const char16_t *p=&TokenRelation[1];i<TOKSREL_SIZE; i++,p++){if(*p==op) return i;}
	return REL_NONE;
}

//La palabra ha de estar terminada por un blanco
//Si no es una keyword devuelve KW_NONE
static KeyWord kw_classify(const char8_t *pc){
	ifz(strbeginsby8(pc,"log")){
		for(KeyWord k=KeyWord_Min; k<KeyWord_logTope; k++){
			ifz(strcmp8_b(pc,KeyWords[k].name)) return k;
		}
	}else{
		for(KeyWord k=KeyWord_logTope; k<KeyWord_Tope; k++){
			ifz(strcmp8_b(pc,KeyWords[k].name)) return k;
		}
	}
	return KW_NONE;
}

//Reemplaza por ' ' los caracteres del token ass al que apunta pc
//y avanza pc a la pos. tras el token. Si no apunta a un Tok_Assign no hace nada
static char8_t * ass_clear(char8_t *pc){
	ifunlike(is_stn0(*pc)) return pc;
	if(*pc=='='){
		*pc++=' ';
		if(*pc=='=') *pc++=' ';
	}elif(pc[1]=='='){
		if(*pc==':' || (*pc>='0' && *pc<='4')) *pc++=' ', *pc++=' ';
	}
	return pc;
}

//Avanzar pc hasta el final de palabra o el token de asignación, lo que aparezca antes.
sinline char8_t *key_advance(const char8_t *pc){
	while(isnot_stn(*pc) && ass_classify(pc)==ASS_NONE) pc++;
	return (ICONST char8_t*)pc;
}

/*isolate_key_advance, isolate_key0_advance
pc apunta a una clave (la primera palabra de una línea) y el resto de la línea.
Avanza pc hasta el principio de la palabra siguiente o el '\n' o COMMENT_CHAR del final de línea
si no hay más palabras.
Return:
	.kw: El código de palabra clave. KW_NONE si no es palabra clave.
	.ass: el Tok_Assign que media entre la clave y el resto de la línea. ASS_NONE si no hay ninguno

    Tras llamar a la función: La primera palabra de la línea, a la que apuntaba pc en la llamada a la
función se ha dejado terminada por ' ' (isolate_key_advance) o por '\0' (isolate_key0_advance). (este
carácter ha podido machacar el token de asignación, =' u otro, que allí hubera) salvo que tras la clave
estuviese inmediatamente el '\n' de final de línea, en cuyo caso la clave queda terminada por '\n'.
    Si el contenido de *pc inicial (la posición de pc al pasarlo a la función), tras la llamada a la función,
es '\n', es porque ya era *pc='\n' al llamar a la función. En caso contrario, si es ' ' (isolate_key_advance)
'\0' (isolate_key0_advance), es porque la línea comenzaba directamente con un "=" u otra asignación,
y si **pc tras llamar a la función es '\n' es que no había nada en la línea tras la clave y/o asignación.
*/
static KwAss isolate_key_advance(char8_t **pc){
	u8int npar; //Nivel de paréntesis
	KwAss r;
	const char8_t *const PTR=*pc;
	r.kw=KW_NONE;
	r.ass=ASS_NONE;

	npar=0;
	//Avanzar pc hasta el final de palabra o el token de asignación, lo que aparezca antes.
	while(**pc!='\n' && (npar>=1 || (isnot_stn(**pc) && (r.ass=ass_classify(*pc))==ASS_NONE))){
		if(**pc=='(') npar++;
		elif(**pc==')' && npar>0) npar--;
		(*pc)++;
	}
	//Mirar si es una palabra clave. Las hay que van seguidas de = y las hay que no.
	{char8_t c=**pc; **pc=' ';
	r.kw=kw_classify(PTR);
	**pc=c;}

	if(r.ass==ASS_NONE && **pc!='\n'){ //Saltar los espacios tras la clave hasta la palabra siguiente...
		**pc=' '; Advanceinline_pc(*pc); //con cuidado de dejar ' ' en el primer blanco.
		//... que es la asignación, en su caso
		if((KeyWords[r.kw].flags&KWFLAG_AssTex)!=KWFLAG_Texto) r.ass=ass_classify(*pc);
	}

	if(r.ass!=ASS_NONE){
		*pc=ass_clear(*pc);
		Advanceinline_pc(*pc);
	}

	return r;
}

/*v. isolate_key_advance*/
//Deja la clave terminada por '\0', salvo que estuviera terminada por '\n', que se mantiene
//*pc queda apuntando a la siguiente palabra de la línea o al '\n' del final si no hay ninguna.
static KwAss isolate_key0_advance(char8_t **pc){
	char8_t *ptr=*pc, c=**pc;
	KwAss r=isolate_key_advance(pc);
	ifunlike(c=='\n') return r;
	ptr=*pc-1;
	ifunlike(!is_st(*ptr)) return r; //Hence, pc ha quedado justo past la palabra, y eso solo es posible si *pc=='\n', el del final de la línea
	do ptr--; while(is_st(*ptr)); *++ptr='\0';
	return r;
}

//Devuelve el Tok_Relation a que apunta pc, terminado por stn
//REL_NONE, si no es relation.
static Tok_Relation is_relation(const char8_t *pc){
	char16_t c;
	const char8_t *end;

	if(pc[1]=='='){ //eq compuesto
		if(!is_stn(pc[2])) return REL_NONE;
		if(*pc=='!') c=u'≠';
		elif(*pc=='<') c=u'≤';
		elif(*pc=='>') c=u'≥';
		else return REL_NONE;
	}else{
		end=c16___u8(&c,pc);
		if(!is_stn(*end)) return REL_NONE;
	}

	return rel_classify(c);
}

//buf->pc apunta al principio de una línea a incorporar. Encuentra el último caracter
//que hay que incorporar y deja buf->pc apuntando al inmediatamente siguiente, que
//puede ser un ' ', '\t', '\n' o '%'.
static void bufline_find_last(Bufferti8_lc *buf){
	char8_t *ppio=buf->pc;
	do{
		str_nC(buf->pc);
		if(*buf->pc==COMMENT_CHAR && isnot_st(buf->pc[-1])){str_stn(buf->pc);}
	}while(MORE_INL(*buf));
	if(buf->pc!=ppio){ //Eliminar los espacios del final de la línea
		do buf->pc--; while(buf->pc!=ppio && is_st(*buf->pc));
		if(isnot_st(*buf->pc)) buf->pc++;
	}
}

/*Lleva a cabo las tareas necesarias cuando buf->pc apunta al '}' (o '\0' en caso de error)
de una multiline definition. Tiene que ser kvs->strs.next[-1]='\n':
    Si buf->pc=='\0': Muestra un mensaje de error e incrementa err_count.
    Else: Avanza buf->pc. Si encuentra ahí una 'n' retrocede kvs->strs.next,
		que quedará apuntando al '\n' último añadido

line: buf->lc guarda la línea en la que se está, pero para informar del error ha de indicarse mejor
la línea donde empezó la definición. Si no se tiene, páses buf->lc ahí.
*/
static void check_end_multiline(Bufferti8_lc *buf, uint line, ErrorOpts *errors, const char8_t *PTR, KeyVals *kvs){
	ifunlike(*buf->pc=='\0'){
		Error_F(errors,line,u8"Runaway argument. En la asignación de %s se ha llegado al final del fichero; falta el '}' de cierre",PTR);
		errors->err_count++;
	}else{
		buf->pc++;
		if(*buf->pc=='n') kvs->strs.next--; //Eliminar el último '\n'
	}
}

//Devuelve el nº de argumento o NO_ARG si no se encuentra
#define NO_ARG 0
static uint8m lookup_one(uint8m nargs, const char8_t *args, char8_t c){
	uint8m a=1;
	while(nargs){
		if(*args==c && args[1]=='\0') return a;
		while(*args!='\0') args++; args++;
		nargs--, a++;
	}
	return NO_ARG;
}

static uint8m lookup_string(uint8m nargs, const char8_t *args, const char8_t *s){
	uint8m a=1;
	while(nargs){
		if(strcmp8(args,s)==0) return a;
		while(*args!='\0') args++; args++;
		nargs--, a++;
	}
	return NO_ARG;
}

/*Reemplaza en s1 cada instancia de #(foo) o #C por #n en donde "foo" o "C" es un nombre
que aparece en la lista arg_names y n es '0'+m, siendo "foo" o "C" el nombre m-ésimo en la
lista, empezando a contar en 1 (e.d., 1º, 2º, etc.). Si "foo" no aparece la cadena #(foo) o #C
desaparece. En este caso se muetra un error. Para esto son los tres últimos argumentos.
*/
static void replace_arg_names(uint8m nargs, const char8_t *arg_names, char8_t *s1, ErrorOpts *errors, uint lc, const char8_t *PTR){
	while(*s1!='\0' && *s1!='#') s1++;
	if(*s1=='\0') return;

	char8_t *p=s1;
	do{ //Here *s1='#'
		uint8m a;
		s1++;
		if(*s1=='#'){*p++='#'; s1++; goto continuar;}
		if(*s1!='('){
			char8_t c=*s1++;
			a=lookup_one(nargs,arg_names,c);
			if(a==NO_ARG){
				Error_F(errors,lc,u8"En el texto de reemplazo de %s se ha encontrado #%c, pero '%c' no se encuentra entre la lista de parámetros de %s.",PTR,c,c,PTR);
				errors->err_count++;
			}
		}else{
			char8_t *parg=s1+1;
			s1=find_closing_paren_n0(s1,NULL);
			if(*s1!=')'){
				Error_F(errors,lc,u8"Runaway argument. En la asignación de %s se ha encontrado un #( sin correspondiente paréntesis de cierre. Todo el texto en la línea a partir de ahí se ignora",PTR);
				errors->err_count++;
				goto continuar;
			}
			*s1++='\0';
			a=lookup_string(nargs,arg_names,parg);
			if(a==NO_ARG){
				Error_F(errors,lc,u8"En el texto de reemplazo de %s se ha encontrado #(%s), pero \"%s\" no se encuentra entre la lista de parámetros de %s.",PTR,parg,parg,PTR);
				errors->err_count++;
			}
		}
		if(a!=NO_ARG){*p++=KV_Substitute; *p++='0'+a;} //Else #c or #(foo) dissapears

	continuar:
		while(*s1!='\0' && *s1!='#') *p++=*s1++;
	}while(*s1!='\0');
	*p='\0';
}
#undef NO_ARG

/* Prepara una línea para su evaluación como op1 rel op2, siendo rel un operador de relación.
    buf->pc apunta a la línea a preparar.
    buf->pc se deja tras la última palabra de la línea (que puede ser el '\n' del final o algo antes)
    En op1, op y op2 se guardarán respectivamente el puntero al operador izquierdo,
el código de operación de relación encontrada y el puntero al operador derecho. Las
cadenas op1 y op2 quedan terminadas por '\n'.
    Se detectará como operador de relación el primero que esté separado por espacios
en blanco de lo que preceda y de lo que siga, o que esté nada más empezar (lo que dará
lugar a un error). Sus espacios alrededor no se incluyen ni en op1 (esp. anteriores) ni
en op2 (esp. posteriores). op1 además se limpia de espacios iniciales; op2 de espacios
finales.
    Si en op2 se devuelve NULL es que ha habido algún error.
    buffer.pc queda en el final de la línea.
*/
static void prepare_rel_line(Bufferti8_lc *buf, char8_t **op1, Tok_Relation *op, char8_t **op2){
	*op=REL_NONE;
	*op2=NULL;
	Advanceinline(*buf); if_Nomore(*buf) return;
	*op1=buf->pc;

	while(MORE_INL(*buf) && (*op=is_relation(buf->pc))==REL_NONE) Ignore_Advanceinline(*buf);
	if(NOMORE_INL(*buf) || *op1==buf->pc) return;
	{char8_t *pc=buf->pc-1; do pc--; while(is_st(*pc)); *++pc='\n';}

	ignore_Advanceinline(*buf); if_Nomore(*buf) return;
	*op2=buf->pc;
	Prepare_line(*buf); resume(*buf);
	if(*buf->pc!='\n') *buf->pc++='\n'; //Deja pc dentro de la línea, de manera
}							//que un ulterior finishline(buffer) funcionará correctamente.

#define HI_LEVEL_IGNORE 0
#define HI_LEVEL_HIGHEST 1
/*Lee y asigna en param un entero pequeño (umint), que puede ir de 0 a max.
errors, PTR, lc: Para mostrar los mensaje de error/warning
bmax: Si el valor encontrado es >max, qué hacer:
	HI_LEVEL_IGNORE: No hacer nada. Se muestra un error.
	HI_LEVEL_HIGHEST: Asignar max. Se muestra un warning

 Si sigue algo más en la línea muestra un warning.
Return:
	0: No se produjo error. *param se asignó.
	1: Se produjo error. *param no se asignó.
*/
static bint read_umint(char8_t **pc, ErrorOpts *errors, uint lc, const char8_t *PTR,umint *param, umint max, bint bmax){
	bint bret;
	uint u=uint___str8(CAST(pc));

	if(isnot_stn(**pc)){
		go_back_pc(*pc);
		Error_F(errors,lc,u8"El valor a asignar a \'%s\' no es un número entero: %Sb",PTR,*pc);
		errors->err_count++;
		return 1;
	}
	bret=0;
	if(u>max){
		//const char8_t * const s=u8"El valor a asignar a \'%s\' es demasiado alto. Puede estar entre 0 y %u. Se encontró %u.";
		if(bmax==HI_LEVEL_IGNORE){
			Error_F(errors,lc,u8"El valor a asignar a \'%s\' es demasiado alto. Puede estar entre 0 y %u. Se encontró %u.",PTR,max,u);
			errors->err_count++;
			bret=1;
		}else{
			Warning_F(errors,lc,u8"El valor a asignar a \'%s\' es demasiado alto. Puede estar entre 0 y %u. Se encontró %u.",PTR,max,u);
			errors->war_count++;
			*param=max;
		}
	}else{
		*param=(umint)u;
	}

	str_no_st(*pc);
	if(**pc!='\n' && **pc!=COMMENT_CHAR){
		Warning_F(errors,lc,"Palabras extra ignoradas tras el comando %s",PTR);
		errors->war_count++;
	}

	return bret;
}

/*s is '\0'-terminated.
La función deja las palabras de s separadas por un único espacio; es decir,
colapsa espacios consecutivos en uno solo. Tras la última palabra no habrá
nada: estará inmediatamete el '\0' de final de cadena (elimina espacios finales).
Si s solamente contenía espacios, tras la función será *s='\0'.
Devuelve el puntero al final de la cadena tras limpiarla.
*/
static char8_t* clean_spaces(char8_t *s){
	char8_t *pc=s;

	//skip initial blanks
	while(*s!='\0' && (*s==' ' || *s=='\t')) s++;
	//advance while pc equals s
	while(pc==s){
		while(*s!='\0' && *s!=' ' && *s!='\t') s++; //go through one word
		pc=s; //pc points at end-of-word
		while(*s!='\0' && (*s==' ' || *s=='\t')) s++; //skip blanks
		if(*s=='\0') break;
		pc++; //There is some other word following the blank(s)
	}
	//if *s!='\0' pc points to where the next (or first) word must start being copied
	//if *s=='\0' pc points to the end of the last word, if any, or the beginning of the string, if none.

	//Loop through the words
	do{
		while(*s!='\0' && *s!=' ' && *s!='\t') *pc++=*s++;
		if(*s=='\0') break;
		while(*s!='\0' && (*s==' ' || *s=='\t')) s++;
		if(*s=='\0') break;
		*pc++=' ';
	}while(1);

	*pc='\0';
	return pc;
}
