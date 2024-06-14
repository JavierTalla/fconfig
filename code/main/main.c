#include <ATcrt/ATfileoutput.h> //Includes AT_filenames.h
#include <ATcrt/definesBufferto8.h>
#include <ATcrt/ATpaths.h>
#include <ATcrt/ATcmdline.h>
#include <ATcrt/ATfiles.h>
#include <stdio.h>
#include <stdbool.h>
#include "../include/fconfig_read.h"
#ifndef ifnzunlike
#define ifnzunlike(x) ifunlike((x)!=0)
#endif
#include "main_utils.c"
#ifndef ifnz
#define ifnz(a) if((a)!=0)
#endif

#if FILE_PREFERRED_ENCODING==FILE_ENCODING_BIT16 && defined(_WIN32)
	#define main wmain
#endif

int main(int _unused(argc), charmain_t **_argv){
	int nret;
	struct OutDir od;
	Bufferto8 blog;
	charfile_t **argv=(charfile_t**)_argv;
	charfile_t** const ARGV=argv; //Para escribir en el log.
	FicheroConfig config;

#ifdef _DEBUG
	getchar();
#endif

	putc('\n',stderr);
#if ARGV0_IS_PROGNAME
	iflike(*argv!=NULL) argv++;
#endif
	if(*argv==NULL){
		PUTerr(u8"Uso del programa:\n"
			u8"fconfig <filename> [params]\n"
			u8"\n"
			u8"<filename>: Nombre del fichero con la extensión.\n"
			u8"Sea <file> el nombre sin la extesión. Se creará un fichero de log, <file>.log.\n"
			u8"Al terminar el procesado se volcarán las variables al fichero <file>_dump.config.\n"
		);
		return 0;
	}

	nret=procesa_filename((charfile_t econst *econst **)&argv, &od);
	if(nret==1) return 0;
	if(nret!=0) return nret;

	ifnz(nret=crea_logfile(&od,&blog)) return nret;
	log_banner(&blog,(charfile_t iconst * iconst *)ARGV);

	if(*argv!=NULL) PUTerr(u8"Argumentos extra en la línea de comandos ignorados\n");

	//Llamar a la librería
	nret=lee_ficheroconfig(&config,od.fout8,&blog);
	toclose(&blog);
	ifnzunlike(nret){PUTerr(u8"No se puede abrir el fichero: "); PUTerrnl(od.fout8); return 5;}
	strcpy8(od.ext,u8".log");
	PUTerr(u8"Terminado. Log escrito en "); PUTerrnl(od.fout8);

	nret=0; /*strcpy8(od.ext,u8"_dump.config");
	nret=dump_keyvals(od.fout8,&config.kvs);
	ifunlike(nret<0){PUTerr(u8"No se pudo abrir el fichero para el volcado de las claves: "); PUTerrnl(od.fout8); nret=6;}
	else ifunlike(nret!=0){PUTerr(u8"Se produjo algún error de escritura en el fichero "); PUTerrnl(od.fout8); nret=0;}
	else{PUTerr(u8"Claves escritas al fichero "); PUTerrnl(od.fout8);}*/
	PUTerr(u8"Volcando las claves a stdout\n\n");
	dump_kvs_online(&config.kvs);
	FicheroConfig_free(&config);
	return nret;
}
