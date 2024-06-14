#pragma once

#define MACRO_OBJECT 0
#define MACRO_FUNC 1
#define MACRO_SPECIAL 2

typedef struct strWhereKey{
	umint macro_type;
	uint n; //Index in the array of MacObj or MacFun, or identifier of special macro
} WhereKey;

#define htype hBase8WhereKey
#define datatype WhereKey
#include <ATcrt/hash_base8data.cod>
#define HsetupBase8WhereKey(hash,base_ptr,n,nomem_code) HsetupЯ(hBase8WhereKey,hash,n,nomem_code); (hash).base=&(base_ptr)

#define VAL_TYPE_unknown	0 //The string has not yet been parsed
#define VAL_TYPE_string		1
#define VAL_TYPE_bool			2
#define VAL_TYPE_uint			3
#define VAL_TYPE_sMedida	4 //Se almacena en dmm
#define VAL_TYPE_float			5

/* Si el valor guardado en un ObjectValue es erróneo (típicamente, como resultado de intentar
parasear un string como un cierto tipo sin éxito o con un error grave), el campo 'type'
será VTYPE_erroneous_<tipo>, que se forma a partir del correspondiente valor bueno:
VAL_TYPE_<tipo>, aplicando un complemento a uno.*/
#define VTYPE_erroneous(x) ((umint)~x) //x must be a normal (non-error) type
#define VTYPE_iserror(x) isneg(x)
#define VTYPE_isright(x) ispos(x)

#define VTYPE_erroneous_bool ((umint)~VAL_TYPE_bool)
#define VTYPE_erroneous_uint ((umint)~VAL_TYPE_uint)
#define VTYPE_erroneous_sMedida ((umint)~VAL_TYPE_sMedida)
#define VTYPE_erroneous_float ((umint)~VAL_TYPE_float)

static const char8_t * const TypesStrings[]={
	[VAL_TYPE_unknown]=u8"none",
	[VAL_TYPE_string]=u8"string",
	[VAL_TYPE_bool]=u8"bool",
	[VAL_TYPE_uint]=u8"entero",
	[VAL_TYPE_sMedida]=u8"medida",
	[VAL_TYPE_float]=u8"float",
};

typedef struct{
	union{bool8 b; uint val; ssint sval; float fl;};
	umint type;
} ObjectValue;

//Campos comunes a una macro obj y a una macro func.
typedef struct{
	u16int file; //File from which it was read
	uint line; //Line in file wherefrom it was read
	uint key; //Position within a block of char8_t-memory. '\0' ended
	uint str; //Position within a block of char8_t-memory. ' ','\0' ended.
	uint strsize; //Size of the string str, not including the ' ', '\0' characters at the end.
	unsigned exp_pending: 1;
	unsigned expding: 1;
} MacroCommon;

/*A defined variable is stored, when it is read, as a double: the variable's name and
its replacement text: str. The string at position strs.ppio+key is terminated by '\0'.
That at position strs.ppio+str is terminated by ' ' '\0'. That last space is not part of str.
    If it be needed, the string is parsed in order to produce a value of a certain type:
string, bool, uint, float or signed measure. See parse_unknown().
   strsize is the size of the string str, not including the closing ' ' '\0'. Thus, at
strs.ppio[str+strsize] is the closing ' '. An empty string has strsize = 0.
*/
typedef struct{
	MacroCommon c;
	ObjectValue value;
} MacObj;

/*Los args se emplean solo para diagnostics. Por ello tenerlos uno a continuación
de otro, necesitando acceso secuencial, está bien.
Dentro de la cadena str, los argumentos se identifican por el carácter KV_Substitute.
Va segudio de '0'+n, donde n es el número de argumento a sustituir. Estos se empiezan
a contar en 1. Ej.:

El valor de 0x1A '1' es 0x1A '2'.

Si esto se invoca como macname(var,354) el reemplazo será
El valor de var es 254.
*/
typedef struct{
	MacroCommon c;
	uint8m nargs; //If nargs is 0 there will be nothing at strs.ppio+args, not even a '\0'.
	uint args; //Names of args. Position within a block of char8_t-memory. Args come one after the other, each one '\0' ended
	uint nreplacements; //How many #(arg) appear in the function's replacement text.
} MacFun;

#define KV_Str_null 0
#define KV_Key_null 1
#define KV_Str_setnull(com) (com)->str=KV_Str_null, (com)->strsize=0

#define MacObj_defaults(kv) (kv).c.key=KV_Key_null, (kv).c.str=KV_Str_null, (kv).c.strsize=0,\
									(kv).value.type=VAL_TYPE_unknown,\
									(kv).c.exp_pending=0, (kv).c.expding=0

#define MacFun_defaults(kv) (kv).c.key=KV_Key_null, (kv).c.str=KV_Str_null, (kv).c.strsize=0,\
									(kv).nargs=0, (kv).args=KV_Str_null,\
									(kv).c.exp_pending=0, (kv).c.expding=0

#define KV_NARGS_MAX 40 //Must be <= 255-'0'
#define KV_BeginExpand 0x0E //Shift Out
#define KV_EndExpand 0x0F //Shift In
#define KV_Substitute 0x1A //Substitute
#define is_KVspecial(c) ((c)==KV_BeginExpand || (c)==KV_EndExpand)

typedef struct strKeyVal{
	umint macro_type;
	union{
		MacObj *obj;
		MacFun *fun;
	} p;
} KeyVal;

defineVector(MacObj)
defineVector(MacFun)

typedef struct{
	Vector_MacObj objs;
	Vector_MacFun funcs;
	Hash_hBase8WhereKey hash;
	Growing_char8_t strs; //Almacena todos los strings
	Vector_uint filenames;
} KeyVals;

#ifndef free_null
#define free_null(x) free(x), x=NULL //From ATmacros.h
#endif
#define KeyVals_free(kv) do{\
	free_null((kv).objs.ppio);\
	free_null((kv).funcs.ppio);\
	free_null((kv).hash.ppio);\
	free_null((kv).strs.ppio);\
	free_null((kv).filenames.ppio);\
}while(0)

typedef struct{
	KeyVals kvs;
	struct{
		u16int err_count;
		u16int war_count;
	} stats;
} FicheroConfig;


/* Estructura para pasar a las funciones que buscan y parsean las claves leídas de un fichero */

#define FConfigParse_ErrOutput_NONE 0 //Nothing
#define FConfigParse_ErrOutput_ERR 1 //Errores
#define FConfigParse_ErrOutput_WAR 2 //Warnings

typedef struct{
	Bufferto8 *blog; //Si no es NULL, donde se escribirán los errores
	uint8m on;		//Si mostrar el error en stdout
	uint8m indent; //Indent deseado al escribir al log
	uint8m err_war; //Opciones de mostrar errores o avisos
	int err; //Latest error code from parsing.
	u16int err_count;
	u16int war_count;
} ErrorOpts;

//El campo kvs es típicamente el resultado de la lectura de un fichero (lee_ficheroconfig).
//El campo errors ha de incializarlo el que llama.
typedef struct{
	KeyVals kvs;
	ErrorOpts errors;
} FConfigParse;

