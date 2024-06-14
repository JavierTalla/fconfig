#define GET_LINE buf->lc

/** Funciones para buffer.pc. Expanden:
    Una línea que puede estar terminada por '\n' o comentario.
    Una palabra, terminada por stn.
    Dejan buffer.pc avanzado.
**/

/*buf->pc apunta a una cadena-nC de texto. Primero elimina los espacios finales de la línea.
Luego expande la línea.
    El resultado queda copiado en expanded, terminado por '\0'.
    Tras la llamada buf->pc queda apuntando al carácter C/n del final de la línea.

 Return:
	Un puntero al interior de expanded, al '\0' que cierra la cadena expandida.
 errors->err:
	0: Todo bien
	AT_NOMEM: Sin memoria
	>0: Algún error al expandir
*/
static char8_t* expand_bufline(Bufferti8_lc *buf, char8_t expanded[MAX_LINE_EX], KeyVals *kvs, ErrorOpts *errors, const strLog *logs){
	if(*buf->pc=='\n' || (*buf->pc==COMMENT_CHAR && is_stn(buf->pc[-1]))){
		errors->err=0; expanded[0]='\0'; return &expanded[0];
	}
	char8_t línea[MAX_LINE_EX];
	char8_t *pn=&línea[0];

	do *pn++=*buf->pc++; while(*buf->pc!='\n' && !(*buf->pc==COMMENT_CHAR && is_st(buf->pc[-1])));
	//Eliminar espacios finales
	pn--; while(pn!=línea && is_st(*pn)) pn--;
	if(isnot_st(*pn)) pn++;
	*pn='\0';
	return exptext_reglas_0(línea,expanded,kvs,errors,GET_LINE,logs);
}

/*buf->pc apunta a una cadena-stn de texto (o a un '\0', en cuyo caso ->pc no se mueve y será expanded[0]='\0').
    El resultado queda copiado en expanded, terminado por '\0'.
    Tras la llamada buf->pc queda apuntando al carácter stn del final de palabla.

 Return:
	Un puntero al interior de expanded, al '\0' que cierra la cadena expandida.
 errors->err:
	0: Todo bien
	AT_NOMEM: Sin memoria
	>0: Algún error al expandir
*/
static char8_t* expand_total_bufword(Bufferti8_lc *buf, char8_t expanded[MAX_LINE_EX], KeyVals *kvs, ErrorOpts *errors, const strLog *logs){
	next_fwd_save(*buf);
	char8_t *pn=exptext_total_0(buf->pc,expanded,kvs,errors,GET_LINE,logs);
	ifunlike(errors->err){ERROR(errors,u8"Error al expandir %s. Se expandió a %s\n",buf->pc,expanded);}
	resume(*buf);
	return pn;
}

/*buf->pc apunta a una cadena-stn de texto (o a un '\0', en cuyo caso ->pc no se mueve y será expanded[0]='\0').
    pc se expande completamente y el resultado queda copiado en expanded, terminado por '\0'.
    Tras la llamada buf->pc queda apuntando al carácter stn del final de palabla.
    La expansión tiene que ser válida como nombre de clave. Si no lo es muestra un mensaje de error
e incrementa errors->err_count.

PTR: Nombre del comando en el que el texto se está expandiendo

 Return:
	Si la expansión es válida: Un puntero al interior de expanded, al '\0' que cierra la cadena expandida.
	Si la expansión no es válida: Un puntero un carácter intermedio en expanded, != '\0'.
 errors->err:
	0: Todo bien
	AT_NOMEM: Sin memoria
	VAR_INVALID_NAME: La expansión no es válida como nombre de variable
	otro >0: Algún error al expandir, o la expansión no es válida
*/
static char8_t* expand_buf_keyword(Bufferti8_lc *buf, char8_t expanded[MAX_LINE_EX], KeyVals *kvs, ErrorOpts *errors, const strLog *logs, const char8_t *PTR){
	next_fwd_save(*buf);
	exptext_total_0(buf->pc,expanded,kvs,errors,GET_LINE,logs);
	ifunlike(errors->err){ERROR(errors,u8"Error al expandir %s. Se expandió a %s\n",buf->pc,expanded);}

	char8_t *pc=expanded; while(isnot_stn0(*pc)) pc++;
	if(*pc!='\0'){
		ERROR(errors,u8"En un comando %s, la expansión de la palabra no es una única palabra (no srive como nombre de clave): %s -> %s",PTR,buf->pc,expanded);
		errors->err_count++;
		errors->err=VAR_INVALID_NAME;
	}
	resume(*buf);
	return pc;
}

/*buf->pc apunta a una cadena-stn de texto (o a un '\0', en cuyo caso ->pc no se mueve y será expanded[0]='\0').
    pc se expande completamente y el resultado queda copiado en expanded, terminado por '\0'.
    Tras la llamada buf->pc queda apuntando al carácter stn del final de palabla.
    La expansión no puede ser vacía y tiene que ser válida como nombre de clave. Si esto no se cumple
muestra un mensaje de error e incrementa errors->err_count.

PTR: Nombre del comando en el que el texto se está expandiendo

 Return:
	Si la expansión es válida: Un puntero al interior de expanded, al '\0' que cierra la cadena expandida.
	Si la expansión es vacía: Un puntero al principio de expanded, que es '\0'
	Si la expansión no es válida: Un puntero un carácter intermedio en expanded, != '\0'.
 errors->err:
	0: Todo bien
	AT_NOMEM: Sin memoria
	VAR_INVALID_NAME: La expansión es vacía o no es válida como nombre de variable
	otro >0: Algún error al expandir, o la expansión no es válida
*/
static char8_t* expand_buf_keyword_noempty(Bufferti8_lc *buf, char8_t expanded[MAX_LINE_EX], KeyVals *kvs, ErrorOpts *errors, const strLog *logs, const char8_t *PTR){
	next_fwd_save(*buf);
	exptext_total_0(buf->pc,expanded,kvs,errors,GET_LINE,logs);
	ifunlike(errors->err){ERROR(errors,u8"Error al expandir %s. Se expandió a %s\n",buf->pc,expanded);}

	char8_t *pc=expanded;
	ifunlike(*expanded=='\0'){
		ERROR(errors,u8"En un comando %s, la expansión de la palabra %s es vacía, tenía que ser un nombre de clave",PTR,buf->pc);
		errors->err_count++; errors->err=VAR_INVALID_NAME;
	}else{
	   while(isnot_stn0(*pc)) pc++;
	   if(*pc!='\0'){
		ERROR(errors,u8"En un comando %s, la expansión de la palabra no es una única palabra (no srive como nombre de clave): %s -> %s",PTR,buf->pc,expanded);
		errors->err_count++; errors->err=VAR_INVALID_NAME;
	}}

	resume(*buf);
	return pc;
}

/*buf->pc apunta a una cadena-nC de texto. Primero elimina los espacios finales de la línea.
Luego expande la línea..
    La línea expandida se copia a expanded, terminado por '\n'. La última palabra de
expanded queda seguida de ' '. Por tanto, si hay al menos una palabra, tras la última
palabra habrá ' ','\n'.
Si la línea estaba VACÍA será *expanded='\n'.

Returns:
	Un puntero al interior de expanded, al '\n' que cierra la cadena expandida.
 errors->err:
	0: Todo bien
	AT_NOMEM: Sin memoria
	>0: Algún error al expandir
*/
sinline char8_t* expand_sn_bufline(Bufferti8_lc *buf, char8_t expanded[MAX_LINE_EX], KeyVals *kvs, ErrorOpts *errors, const strLog *logs){
	char8_t *line=expand_bufline(buf,expanded,kvs,errors,logs);
	*line++=' '; *line='\n';
	return line;
}

/*buf->pc queda apuntando al carácter C/n del final de la línea.
La línea expandida se copia a expanded, terminado por '\0'. Elimina los espacios finales
de la línea (antes de expandir).
Returns:
	0: Todo bien
	VARS_SYNTAX:	Error sinctáctico
	VARS_RUNAWAY:	Runaway argument
	EXPANSION_RECURSIVE: Eso

En caso de error la función escribe al log el correspondiente mensaje e incrementa err_count
*/
static int expandlog_fileline(Bufferti8_lc *buf, Globals *globals, const char8_t *PTR, char8_t expanded[MAX_LINE_EX]){
	const char8_t *line=buf->pc;
	expand_bufline(buf,expanded,&globals->kvs,&globals->errors,&globals->log);
	ifunlike(globals->errors.err!=0){ERROR(&globals->errors,u8"Al expandir la línea %s %Sn",PTR,line);}
	return globals->errors.err;
}

/*buf->pc queda apuntando al carácter C/n del final de la línea.
La línea expandida se copia a expanded, terminado por '\n'. Elimina los espacios finales
de la línea (antes de expandir). La última palabra de expanded queda seguida de ' '.
Por tanto, si hay al menos una palabra, tras la última palabra habrá ' ','\n'.
Si la LÍNEA estaba VACÍA será *expanded='\n'.
Returns:
	0: Todo bien
	VARS_SYNTAX:	Error sinctáctico
	VARS_RUNAWAY:	Runaway argument
	EXPANSION_RECURSIVE: Eso

En caso de error la función escribe al log el correspondiente mensaje e incrementa err_count
*/
static int expandlog_bufline(Bufferti8_lc *buf, Globals *globals, const char8_t *PTR, char8_t expanded[MAX_LINE_EX]){
	const char8_t *line=buf->pc;
	expand_sn_bufline(buf,expanded,&globals->kvs,&globals->errors,&globals->log);
	iflike(globals->errors.err==0) return 0;
	wrong_line_0n(&globals->errors,PTR,line);
	return globals->errors.err;
}

#undef GET_LINE
