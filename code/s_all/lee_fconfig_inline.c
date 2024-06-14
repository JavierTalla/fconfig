static char8_t name_scratch[SHRT_PATH];

#define RE_PARSE 2
//Si devuelve RE_PARSE es para que inmediatamente se llame otra vez a esta función con el fichero
//escrito en name_scratch. Esto sucede por una orden jump.
static int lee_ficheroconfig_buffer(Bufferti8_lc buffer, Globals *globals, const char8_t *fconfig);

/*fconfig tiene que venir en utf8.
Si el fichero se puede abrir y hay log se escribe ahí la indicación de que se abre y, cuando se cierre, de que se cierra.
Escribe al log mensajes según log_level. errors.err_count y errors.war_count se incrementan a cada error/warning encontrado.
Return:
	0: Todo bien
	AT_NOMEM: Sin memoria
	Otro <0: Uno de los códigos definidos en ATfileinput.h por no poder abrir el fichero*/
#define LEEFICH_STOP 1 // Se ha detenido por una orden stop.
/*
globals->state.file_lev ha de ser 0 en una llamada a esta función desde fuera.
globals->state.nfile tiene que venir asignado. Es quien llama quien decide el número de fichero
*/
sinline int lee_ficheroconfig_inline(const char8_t* fconfig, Globals *globals){
	int nret;
	Bufferti8_lc buffer;
	ifunlike((nret=tiopen_utf8((Bufferti8*)&buffer,fconfig))<0) return nret;
	nret=lee_ficheroconfig_buffer(buffer,globals,fconfig);
	while(nret==RE_PARSE){
		ifunlike((nret=tiopen_utf8((Bufferti8*)&buffer,name_scratch))<0) return nret;
		nret=lee_ficheroconfig_buffer(buffer,globals,name_scratch);
	}
	return nret;
}

#define GET_LINE line
static void LOGassign(Globals *globals,uint line, const KeyVal *kv, bint special, Tok_Assign ass){
	bint bstr=0; //Wether to output or not the com.str string.
	const char8_t *skey=KeyVal_key(&globals->kvs,kv);
	if(special){
		LOG(assigns,LASSIGNS_SPECIALS,sss,skey,u8" ",TokenAssignment[ass]);
		if(isLevel(assigns,LASSIGNS_SPECIALS)) bstr=1;
	}else{
		LOG(assigns,LASSIGNS_REGULARS_KEY,sss,skey,u8" ",TokenAssignment[ass]);
		if(isLevel(assigns,LASSIGNS_REGULARS_KEYVAL)) bstr=1;
		elif(isLevel(assigns,LASSIGNS_REGULARS_KEY)){
			if(globals->errors.blog!=NULL) toput_char(globals->errors.blog,'\n');
			if(globals->errors.on) putc('\n',stderr);
		}
	}
	if(!bstr) return;

	if(kv->macro_type==MACRO_OBJECT){
		if(globals->errors.blog!=NULL){
			toput_char(globals->errors.blog,' ');
			dump_macobj_val(globals->errors.blog,kv->p.obj,&globals->kvs,globals->errors.indent);
		}
		if(globals->errors.on){
			putc(' ',stderr);
			dump_macobj_val_stderr(kv->p.obj,&globals->kvs,globals->errors.indent);
		}
	}elif(kv->macro_type==MACRO_FUNC){
		if(globals->errors.blog!=NULL){
			toput_char(globals->errors.blog,' ');
			dump_macfun_val(globals->errors.blog,kv->p.fun,&globals->kvs,globals->errors.indent);
		}
		if(globals->errors.on){
			putc(' ',stderr);
			dump_macfun_val_stderr(kv->p.fun,&globals->kvs,globals->errors.indent);
		}
	}
}
#undef GET_LINE
#define GET_LINE buffer.lc

static void init_local_state(LocalState *l){
	l->ifS.lev=0;
	l->ifS.m=0;
	l->ifS.state=IFState_True;
}

/******---------        LECTURA DEL FICHERO        ---------******/

static void initial_tasks(Globals *globals, const char8_t *fconfig, Bufferti8_lc *buffer, LocalState *lstate, char8_t ruta_this[SHRT_PATH], char8_t active_file[SHRT_PATH]){
	log_openfile(globals,fconfig);
	Bufferti_lc_setup(*buffer);
	globals->state.file_lev++;
	globals->errors.indent=globals->state.file_lev;
	init_local_state(lstate);

	if(fconfig!=NULL){
		strncpy8(active_file,fconfig,SHRT_PATH); active_file[SHRT_PATH-1]='\0';
		strncpy8(ruta_this,fconfig,SHRT_PATH); ruta_this[SHRT_PATH-1]='\0';
		path_remove_file8(ruta_this);
	}else{
		Path_this(active_file);
		Path_this(ruta_this);
	}
}

//If stoped el fichero se dejó de leer y el mensaje es "fin de la lectura... ". En caso contrario el mensaje es "fin del archivo... ".
#define CLOSE_FILE(stopped) \
	ticlose(buffer); \
	globals->state.file_lev--; \
	globals->errors.indent=globals->state.file_lev; \
	log_closefile(globals,fconfig,stopped);

#undef VectorGrowingFactor
#define VectorGrowingFactor 0

//Labels usadas en la función. Para que se coloreen
#define entry_in_line entry_in_line
#define keyval_assign keyval_assign
#define continuar continuar
#define ignorar_línea ignorar_línea

/*fconfig tiene que venir en utf8.
Si hay log se escribe ahí la indicación de que el fichero se ha abierto y, cuando se cierre, de que se cierra.
Si fconfig es NULL escribe "leyendo fichero anónimo" o algo semejante.
Escribe al log mensajes según log_level. errors.err_count y errors.war_count se incrementan a cada error/warning encontrado.
Return:
	0: Todo bien
	AT_NOMEM: Sin memoria

globals->state.file_lev ha de ser 0 en una llamada a esta función desde fuera.
globals->state.nfile tiene que venir asignado. Es quien llama quien decide el número de fichero
*/
static int lee_ficheroconfig_buffer(Bufferti8_lc buffer, Globals *globals, const char8_t *fconfig){
	char8_t ruta_this[SHRT_PATH],
			    active_file[SHRT_PATH]; //Empieza igual a fconfig. Un comando FILE lo puede cambiar
	char8_t expanded[MAX_LINE_EX]; //Un buffer para la línea con las variables expandidas
	char8_t expanded2[MAX_LINE_EX]; //Otro
	LocalState lstate;

	initial_tasks(globals,fconfig,&buffer,&lstate,ruta_this,active_file);

	Advance(buffer);
	goto entry_in_line;
continuar:
	finishline_Advance(buffer);
entry_in_line:
	if(*buffer.pc=='\0') goto end;
	{
		KwAss kwass; //Keyword and assignment token, if any, that ended the key name.
		const char8_t *PTR; //PTR queda apuntando al inicio de la clave
		bint bwrong=0;

		PTR=buffer.pc;
		kwass=isolate_key0_advance(&buffer.pc);
		//*PTR=='\n' es imposible porque no llamamos a la función en líneas vacías.
		if(*PTR=='\0') goto ignorar_línea; //Línea que comienza directamente con "=" u otro token de assignment.
		if(lstate.ifS.state!=IFState_Ready && lstate.ifS.state!=IFState_Waiting){
			if(kwass.kw==KW_NONE && kwass.ass==ASS_NONE && NOMORE_INL(buffer)) goto ignorar_línea; //clave \n
			if(!kwass_isvalid(kwass)){
				globals->errors.err_count++;
				if(kwass.ass==ASS_NONE){
					if(kwass.kw!=KW_NONE){ERROR(&globals->errors,u8"Se encontró \"%s %Sb\". Tras %s se requiere un =.",PTR,buffer.pc,PTR);}
					else{ERROR(&globals->errors,u8"Se encontró \"%s %Sb\". Tras un nombre de clave se requiere una asignación.",PTR,buffer.pc);}
					goto continuar;
				}
				ERROR(&globals->errors,u8"La palabra clave %s no puede ir seguida de %s",PTR,TokenAssignment[kwass.ass]);
				bwrong=1;
			}
		}
		if(lstate.ifS.lev==0){
			if(kwass.kw==KW_Endif || Iftype[kwass.kw]==IFTYPE_ELSE){
				bwrong=1;
				if(kwass.kw==KW_Endif) ERROR_s(&globals->errors,u8"Endif encontrado fuera de todo condicional");
				else ERROR_s(&globals->errors,u8"Else encontrado fuera de todo condicional");
				globals->errors.err_count++;
			}
		}
		if(bwrong) goto continuar;
		if(NOMORE_INL(buffer) && (KeyWords[kwass.kw].flags&KWFLAG_Somethig)) goto ignorar_línea; //No hay valor tras la clave

		#include "IfElse.cod"
		if(kwass.kw<KeyWord_Min || Iftype[kwass.kw]!=IFTYPE_NONE) goto keyval_assign;
		/* Comandos */
		#include "include.cod"
		#include "diagnostics.cod"
		#include "key_handling.cod"
		#include "atend.cod"
		#include "comandos_otros.cod"
		#include "logging.cod" //Tiene que ser el último
		assert(0); //No puede quedar ninguna palabra clave sin tratar

	keyval_assign:
		#include "keyval_assignment.cod" //Comprueba si el nombre es válido. Si no lo es, goto ignorar_línea.
		goto continuar;

	ignorar_línea:
		buffer.savedchar=*buffer.pc; *buffer.pc='\0'; //Para el caso en el que la línea contenía una única palabra seguida
		WARNING(&globals->errors,u8"Línea que comienza por %s%s ignorada",PTR,TokenAssignment[kwass.ass]);
		*buffer.pc=buffer.savedchar; //inmediatamente por el '\n'
		globals->errors.war_count++;

		goto continuar;

	error_y_continuar:
		globals->errors.err_count++; goto continuar;
	warning_y_continuar:
		globals->errors.war_count++; goto continuar;
	}

	int nret;
end:
	//Comprobaciones al terminar todos los ficheros
	if(lstate.ifS.lev!=0){
		Warning_s(&globals->errors,NO_LINE,u8"Fichero terminado sin haberse cerrado todos los condicionales. Falta algún \"Endif\"");
	}
	nret=0; goto salir;
end_stop:
	nret=LEEFICH_STOP; goto salir;
salida_outofmem:
	nret=AT_NOMEM; goto salir;

salir:
	CLOSE_FILE(nret!=0)
	return nret;
}

#undef GET_LINE
