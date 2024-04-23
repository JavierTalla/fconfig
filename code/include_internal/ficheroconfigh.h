#define sinline static inline
#define elif else if
#define elifz else ifz
#define elifunlike else ifunlike
#include "configfile_syntax.h"

//Obtener el puntero al file, key o str del keyval kv.
#define KeyKeyg(kv) KeyKey(globals,kv)
#define KeyStrg(kv) KeyStr(globals,kv)
#define KeyArgsg(kv) KeyArgs(globals,kv)

//Obtener el puntero al file, key o str del keyval kv.
#undef KeyStrp
#undef KeyKeyp
//#undef KeyArgsp
#define KeyStrp(kv) (kvs->strs.ppio+kv->c.str)
#define KeyKeyp(kv) (kvs->strs.ppio+kv->c.key)
//#define KeyArgsp(kv) (kvs->strs.ppio+kv->args)

//Número máximo de variables que se pueden definir en un fichero que se lee
#define VARNAME_MAX 100 //Longitud máxima de un nombre de variable
#define MAX_LINE_EX 800 //Longitud máxima de una línea ya expandida

#define LCOMMANDS 1 //Ir indicando los comandos:
#define LCOMMANDSALL 2 //Ir indicando los comandos, incluso 'echo' 'copyroot' y otros que normalmente no se indican
//branches. 2: Line after expansion

#define LASSIGNS_SPECIALS 1 //Mostrar solamente la asignación de variables especiales
#define LASSIGNS_REGULARS_KEY 2 //Mostrar los nombres de las variables que se van asignando
#define LASSIGNS_REGULARS_KEYVAL 3 //Mostrar también los valores

#define SetLogNone(log) (log)=(strLog){0}
#define SetLogAll(log) oneset(&(log).fich,usizeof(strLog)-uoffsetof(strLog,fich))
#define SetLogDefault(log) SetLogNone(log), log.fich=1, (log).assigns=LASSIGNS_SPECIALS
//
#define SetStrLogDefault(log) SetLogDefault(log), (log).formatmsg=1

//Estructura que se pasan las funciones que leen el fichero de configuración
typedef struct{
	Bufferto8 *BLOG; //Puntero permanente al fichero log
	strLog log;
	KeyVals kvs;
	ErrorOpts errors; //Su campo .log será =BLOG o NULL, según el logging esté on or off.
	InterpreterState state;
	Growing_char8_t atend; //Buffer con código para leer al final
} Globals;

#define isKW(KW) (kwass.kw==KW)
#define ifKW(KW) if(kwass.kw==KW)

#define CAST(_pe) (econst char8_t**)(_pe)
#define egetu(type,u) do{u=(type)uint___str8(CAST(&pe)); str_no_st(pe);}while(0) //unsigned

//Añadir un carácter a globals->kvs.str
#define Addstr_c(c) Gadd(globals->kvs.strs,char8_t,c,goto salida_outofmem);
//Añadir una cadena con end apuntando al '\0' de cierre
#define Addstr_end(s,end) GC8_adds_end(globals->kvs.strs,s,end,goto salida_outofmem);
//Añadir a globals->kvs.str una cadena terminada por '\0', etc.
#define Addstr_0(s) GC8_adds_0(globals->kvs.strs,s,goto salida_outofmem)
#define Addstr_n(s) GC8_adds_n(globals->kvs.strs,s,goto salida_outofmem)
#define Addstr_b(s) GC8_adds_b(globals->kvs.strs,s,goto salida_outofmem)
#define Addstr_st(s) GC8_adds_st(globals->kvs.strs,s,goto salida_outofmem)
//Añadir a globals->kvs.str el carácter de cierre de una cadena recién añadida
#define Addstr_close(c) *globals->kvs.strs.next++=c
