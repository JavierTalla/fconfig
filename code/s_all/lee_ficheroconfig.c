void Globals_free(Globals *globals){
	free_null(globals->atend.ppio);
	KeyVals_free(globals->kvs);
}

/*Inicializa globales.
Reserva espacio para nfiles en globals.kvs.filenames, nkeys claves en globals.kvs.arr
y para nstr_chars caracteres en globals.kvs.strs. Reserva también para el buffer atend.
    Si en alguna de esas reservas de memoria no hay memoria, libera todo, deja los punteros
a NULL y devuelve AT_NOMEM. Devuelve 0 en caso favorable.

blog y BLOG se dejan a NULL.
En el array globals.kvs.strs se añade una cadena vacía al principio; es decir, se añade ' ', '\n'.
*/
int Globals_setup(Globals *globals, uint nfiles, uint nobjs, uint nfuncs, uint nstr_chars){
	*globals=(Globals){0};
	SetStrLogDefault(globals->log);
	globals->errors.err_war=LWAR;
	globals->state.nfile=0;
	globals->state.file_lev=0;
	globals->state.atend_nfile=-1;
	globals->state.override=0;

	if(nfiles!=0){
		maxeq(nfiles,4);
		Vsetup(uint,globals->kvs.filenames,nfiles,goto salida_outofmem);
	}
	if(nobjs!=0){
		maxeq(nobjs,4);
		Vsetup(MacObj,globals->kvs.objs,nobjs,goto salida_outofmem);
	}
	if(nfuncs!=0){
		maxeq(nfuncs,2);
		Vsetup(MacFun,globals->kvs.funcs,nfuncs,goto salida_outofmem);
	}
	{uint nkeys=nobjs+nfuncs;
	if(nkeys!=0){
		HsetupBase8WhereKey(globals->kvs.hash,globals->kvs.strs.ppio,nkeys,goto salida_outofmem);
	}}
	if(nstr_chars!=0){
		maxeq(nstr_chars,4);
		Growing_setup(char8_t,globals->kvs.strs,nstr_chars,goto salida_outofmem);
		Gadd_blind(globals->kvs.strs,' '); Gadd_blind(globals->kvs.strs,'\0'); //Permanent empty string
	}
	GC8_initialize(globals->atend,12, goto salida_outofmem);
	return 0;

salida_outofmem:
	Globals_free(globals);
	return AT_NOMEM;
}

//No checks for out-of-men
static void AddVar_scratch(Globals *globals, const char8_t *key, const char8_t *val){
	MacObj *kv=findcreate_obj_0(&globals->kvs,key);
	Growing_char8_t *strs=&globals->kvs.strs;

	char8_t *old=strs->next;
	kv->c.str=(pdif)(strs->next-strs->ppio);
	strs->next=strpcpy8(strs->next,val);		kv->c.strsize=(pdif)(strs->next-old);
	*strs->next++=' '; *strs->next++='\0';
}

static void AddGlobalVars(Globals *globals){
#ifndef SYSTEM
#define STRSYS "None"
#else
#define EEXXSS(x) u8""#x
#define EXPSYS(x) EEXXSS(x)
#define STRSYS EXPSYS(SYSTEM)
#endif
	AddVar_scratch(globals,u8"SYSTEM",STRSYS);
#undef EEXXSS
#undef EXPSYS
#undef STRSYS
}

#define GET_LINE NO_LINE
//Obtener el puntero a la str del keyval kv.
#undef KeyKeyg
#undef KeyStrg
#define KeyKeyg(kv) (globals.kvs.strs.ppio+kv->c.key)
#define KeyStrg(kv) (globals.kvs.strs.ppio+kv->c.str)
int lee_ficheroconfig(FicheroConfig *config, const char8_t* fconfig, Bufferto8 *log){
	Globals globals;
	if(config!=NULL) *config=(FicheroConfig){0};

	ifunlike(Globals_setup(&globals,20,100,50,2000)==AT_NOMEM) return AT_NOMEM;
	globals.errors.blog=globals.BLOG=(Bufferto8*)log;
	AddGlobalVars(&globals);

	globals.kvs.filenames.ppio[0]=(pdif)(globals.kvs.strs.next-globals.kvs.strs.ppio);
	globals.kvs.filenames.n=1;
	GC8_adds0(globals.kvs.strs,fconfig,goto salida_outofmem);

	#define globals (&globals)
	LOG(fich,1,ssnl,u8"> Buscando el archivo de configuración ",fconfig);
	#undef globals
	{int nret=lee_ficheroconfig_inline(fconfig,&globals);
	ifnz(nret < 0){
		ifunlike(nret==AT_NOMEM) goto salida_outofmem;
		#define globals (&globals)
			if(nret==ATFILEI_BADPATH || nret==ATFILEI_NOFILE) LOG(fich,1,s,u8"Error:  ... no encontrado\n\n");
			else LOG(fich,1,s,u8"Error:  ... no se ha podido abrir\n\n");
		#undef globals
		if(globals.state.nfile!=0){ //El error se produjo en un fichero invocado por 'jump' desde el fichero principal
			globals.errors.err_count++;
			goto continuar;
		}
		if(config!=NULL) config->stats.err_count=1;
		Globals_free(&globals);
		return globals.errors.err;
	}}

continuar:;
	//Procesado de atend
#define AtEnd globals.atend
	uint nend=0;
	Bufferti8_lc buffer;
	while(AtEnd.next!=AtEnd.ppio){
		++nend;
		Gadd(AtEnd,char8_t,'\n',goto salida_outofmem);
		#define globals (&globals)
			if(nend==1) LOG(fich,1,s,u8"> Procesado del código enviado a AtEnd\n");
			else LOG(fich,1,f,u8"> %uª expansión de AtEnd\n",nend);
		#undef globals
		ifunlike(tiopen_mem8((Bufferti8*)&buffer,globals.atend.ppio,(pdif)(AtEnd.next-AtEnd.ppio))
					==AT_NOMEM) goto salida_outofmem;
		AtEnd.next=AtEnd.ppio;
		globals.state.nfile=0;
		globals.state.file_lev=0;
		ifunlike(lee_ficheroconfig_buffer(buffer,&globals,NULL) == AT_NOMEM) goto salida_outofmem;
	}
#undef AtEnd

	//Expansión final
	{durchVectorp(MacObj,globals.kvs.objs){
		const char8_t *old=KeyStrg(p);
		fullexpand_kv(&globals.kvs,p,&globals.errors,NO_LINE,&globals.log);
		ifunlike(globals.errors.err==AT_NOMEM) goto salida_outofmem;
		ifnzunlike(globals.errors.err){
			wrong_var_00(&globals.errors,KeyKeyg(p),old);
		}
	}}
	{durchVectorp(MacFun,globals.kvs.funcs){
		const char8_t *old=KeyStrg(p);
		globals.errors.err=0; //Remove when the next line is programmed
		//fullexpand_kv(&globals.kvs,p,&globals.errors,NO_LINE,&globals.log);
		ifunlike(globals.errors.err==AT_NOMEM) goto salida_outofmem;
		ifnzunlike(globals.errors.err){
			wrong_var_00(&globals.errors,KeyKeyg(p),old);
		}
	}}

	free(globals.atend.ppio);
	if(config!=NULL){
		config->stats.err_count=globals.errors.err_count;
		config->stats.war_count=globals.errors.war_count;
		config->kvs=globals.kvs;
		config->kvs.hash.base=&config->kvs.strs.ppio;
	}else{
		Globals_free(&globals);
	}
	return 0;

salida_outofmem:
	Globals_free(&globals);
	return AT_NOMEM;
}
#undef KeyKeyg
#undef KeyStrg
#undef GET_LINE
