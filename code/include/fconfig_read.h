//Fichero fichero_read.h. Fichero de inclusión para emplear la librería ficheroconfig.
#include <ATcrt/ATcrt_types.h>
#include <ATcrt/ATvisibility.h>
#include <ATcrt/ATarrays.h>

#ifndef COMPILING_FICHEROCONFIG
#define ficheroconfig_api set_visibility(imported)
#define FICHEROCONFIG_API VISIBILITY_BLOCK(imported)
#else
#define ficheroconfig_api set_visibility(exposed)
#define FICHEROCONFIG_API VISIBILITY_BLOCK(exposed)
#endif

#define FCONFIG_VERSION "1.1"
#include "str_fconfig.h"

//Ha de ser un valor < que todos los de magnitudes.h
#define KEY_NOT_FOUND -100
#define ERR_VAL_StringTooLong ERR_VAL_HUGE

FICHEROCONFIG_API
#ifdef __cplusplus
extern "C"{
#endif

/*Lectura de un fichero de configuración, guardando lo leído en una estructura FicheroConfig

config: Puntero a una estructura FicheroConfig que se rellenará. Puede ser NULL
fconfig: Nombre del archivo, en utf-8
log: Si se emplea la librería ATcrt se puede pasar aquí un puntero Bufferto8*, que ha de ser un fichero
	ya abierto en el que la función escribirá el progreso de la lectura. Si no, se ha de pasar NULL.

Return:
    0: Todo bien
    AT_NOMEM: Sin memoria
    <0: No se pudo abrir el fichero fconfig. Uno de los códigos de error definidos en la librería ATcrt
	#define ATFILEI_UNKNOWN -1		//none of the following
	#define ATFILEI_BADPATH -3			//invalid path
	#define ATFILEI_NOFILE -4				//El fichero no existe
	#define ATFILEI_NOMORE -5			//cannot open more files
	#define ATFILEI_NOTALLOWED -6	//no permission for reading that file
	#define ATFILEI_LARGEFILE -7		//>=(2GB+ATFILEMIN) files are not allowed
	#define ATFILEI_MIN -12

Si config no es NULL y si el valor devuelo es 0, al terminar la lectura del fichero 'fconfig' rellena
la estructura 'config'. Incluye punteros a memoria que habrá que liberar llamarndo a
a la función FicheroConfig_free.
*/
extern int lee_ficheroconfig(FicheroConfig *config, const char8_t* fconfig, Bufferto8 *log);

/* file tiene que venir en utf8.
Returns:
    0
    !=0: Uno de los códigos definidos en ATfileoutput:
	//Errores al intentar abrir el fichero
	#define AT_NOMEM -2	//out of memory
	#define ATFILEO_OPEN_UNKNOWN 1		//none of the following
	#define ATFILEO_OPEN_BADPATH 2		//invalid path
	#define ATFILEO_OPEN_NOMORE 3			//cannot open more files
	#define ATFILEO_OPEN_NOTALLOWED 4

	//Errores de escritura
	ATFILEO_ERROR_OTHER 5
	ATFILEO_ERROR_NOSPACE 6
*/
extern int dump_kvs(const char8_t *file, KeyVals *kvs);
extern int dump_kvs_online(KeyVals *kvs); //Always returns 0

static inline void FicheroConfig_free(FicheroConfig *config){
	KeyVals_free(config->kvs);
}

#ifdef __cplusplus
}
#endif
VISIBILITY_BLOCK_END
