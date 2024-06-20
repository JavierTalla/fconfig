#define BANNER u8"Programa 'ficheroconfig', parseador de ficheros fconfig, " __DATE__

/* Logging */
#if FILE_PREFERRED_ENCODING==FILE_ENCODING_BIT8
	#if FILE8BITS_ARE_UTF8
	#define towrite_filename towrite_string
	#else
	#define towrite_filename towrite_string_utf8
	#endif
#else
	#define towrite_filename towrite_string16_utf8
#endif

//For the ensuing arguments
#if FILE_PREFERRED_ENCODING==FILE_ENCODING_BIT8
	#define path_clean path_clean8
#else
	#define path_clean path_clean16
#endif

//#define PUTerr(s)  fputs(s,stderr)
//To enforce type checking
static inline void PUTerr(const char8_t *s){fputs((const char*)s,stderr);}
#define PUTerrnl(s) PUTerr(s), fputc('\n',stderr)

struct OutDir{
	char8_t fout8[SHRT_PATH], *ext;
};

/*  Compone el nombre del fichero consumiendo cuantas palabras de argv sea necesario.

Return:
	0: todo bien
	1: Se ha indicado un nombre de fichero vacío
	2: El nombre indicado para el fichero no es el de un fichero

Escribe los mensajes de error / avisos que sea necesario.
Si el valor devuelto es 0 copia el nombre del fichero a od->fout8, en utf8, y deja
od->ext apuntando al punto de la extensión o al '\0' del final si no hay extensión.
*/
static int procesa_filename(charfile_t const *const **argv, struct OutDir *od){
	int nret;
	charfile_t fout[SHRT_PATH];

	nret=cmdline_compose(argv,fout,NULL);
	if(nret==1) PUTerr(u8"Warning: No hay una comilla de cierre correspondiente a la comilla \" de apertura."
							" Se entiende que el nombre del fichero se extiende hasta el final de la línea de comandos.\n");
	ifunlike(fout[0]==0){
		PUTerr(u8"Se ha indicado un nombre de fichero vacío. Nada que hacer\n");
		return 1;
	}
	nret=0;

	path_clean(fout);
	if(fileclass(fout)!=ATFILETYPE_FILE) nret=2;
#if FILE_PREFERRED_ENCODING==FILE_ENCODING_BIT8
	strcpy8(od->fout8,fout);
#else
	stru8___str16(od->fout8,fout);
#endif
	if(nret){
		PUTerr(u8"No existe ningún fichero de nombre "); PUTerrnl(od->fout8);
		return 2;
	}
	path_get_extension8(od->fout8,od->ext);
	return 0;
}

/*  Crea el fichero de log.

Basándose en ese nombre od crea el nombre del fichero de log y lo abre.
Return:
	0: Todo bien
	4: No se puede abrir el fichero de log

En los casos de error muestra el correspondiente mensaje.
*/
static int crea_logfile(const struct OutDir *od, Bufferto8 *blog){
	char8_t flog8[SHRT_PATH];

	strcpy8(flog8,od->fout8);
	{char8_t *s=flog8+(od->ext-od->fout8);
	*s++='.'; *s++='l'; *s++='o'; *s++='g'; *s='\0';}

	ifnzunlike(toopen_utf8(blog,flog8)){
		PUTerr(u8"No se puede abrir el fichero de log: "); PUTerrnl(flog8);
		return 4;
	}
	return 0;
}

//*argv cannot be NULL
static void log_banner(Bufferto8 *blog, charfile_t const *const *argv){
	PUTerr(BANNER "\n\n");
	towrite_string(blog,BANNER "\n\n");
	towrite_string(blog,u8"Invocación del programa:\n\n");
	goto first;
	do{toput_char(blog,' ');
		first: towrite_filename(blog,*argv); argv++;
	}while(*argv!=NULL);
	toput_char(blog,'\n'); toput_char(blog,'\n');
}
