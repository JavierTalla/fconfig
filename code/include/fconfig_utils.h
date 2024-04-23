//Fichero fconfig_utils.h. Fichero de inclusión para manejar la estructura devuelta por la libería ficheroconfig.
#include <ATcrt/ATcrt_types.h>
#include <ATcrt/ATvisibility.h>
#include <ATcrt/ATarrays.h>
#include <ATcrt/ATfunctions.h>
#include <ATcrt/ATmacros.h>
#include <stdbool.h>

#ifndef COMPILING_FICHEROCONFIG
#define ficheroconfig_api set_visibility(imported)
#define FICHEROCONFIG_API VISIBILITY_BLOCK(imported)
#else
#define ficheroconfig_api set_visibility(exposed)
#define FICHEROCONFIG_API VISIBILITY_BLOCK(exposed)
#endif

#define FCONFIG_VERSION "1.1"
#include "str_fconfig.h"
#include "magnitudes.h"
#include "parsekv.h"

//Ha de ser un valor < que todos los de magnitudes.h
#define KEY_NOT_FOUND -100
#define KEY_NOTOBJECT -101
#define ERR_VAL_StringTooLong ERR_VAL_HUGE

FICHEROCONFIG_API
#ifdef __cplusplus
extern "C"{
#endif

static inline const char8_t *KeyVal_key(const KeyVals *kvs, const KeyVal *kv){
	uint n;
	if(kv->macro_type==MACRO_OBJECT) n=kv->p.obj->c.key;
	else n=kv->p.fun->c.key;
	return kvs->strs.ppio+n;
}

static inline MacroCommon* Com___WhereKey(const KeyVals *kvs, const WhereKey *wk){
	if(wk->macro_type==MACRO_OBJECT) return &kvs->objs.ppio[wk->n].c;
	if(wk->macro_type==MACRO_FUNC) return &kvs->funcs.ppio[wk->n].c;
	return NULL;
}

//Obtener el puntero al file, key o str del MacObj o MacFun kv.
//FicheroConfig *fconfig, MacObj* kv
#define KeyFile(fconfig,kv) ((fconfig)->kvs.strs.ppio+(fconfig)->kvs.filenames.ppio[(kv)->c.file])
#define KeyLine(fconfig,kv) (kv)->c.line
#define KeyKey(fconfig,kv) ((fconfig)->kvs.strs.ppio+(kv)->c.key)
#define KeyStr(fconfig,kv) ((fconfig)->kvs.strs.ppio+(kv)->c.str)
#define KeyArgs(fconfig,kv) ((fconfig)->kvs.strs.ppio+(kv)->args)

//Obtener el puntero al file, key o str de la parte común com
//FicheroConfig *fconfig, MacroCommon* com
#define ComFile(fconfig,com) ((fconfig)->kvs.strs.ppio+(fconfig)->kvs.filenames.ppio[(com)->file])
#define ComLine(fconfig,com) (com)->line
#define ComKey(fconfig,com) ((fconfig)->kvs.strs.ppio+(com)->key)
#define ComStr(fconfig,com) ((fconfig)->kvs.strs.ppio+(com)->str)

//Obtener el puntero al file, key o str del keyval kv.
//KeyVals *kvs, MacObj* kv
#define KeyFilep(kvs,kv) ((kvs)->strs.ppio+(kvs)->filenames.ppio[(kv)->c.file])
#define KeyLinep(kvs,kv) (kv)->c.line
#define KeyStrp(kvs,kv) ((kvs)->strs.ppio+(kv)->c.str)
#define KeyKeyp(kvs,kv) ((kvs)->strs.ppio+(kv)->c.key)
#define KeyArgsp(kvs,kv) ((kvs)->strs.ppio+(kv)->args)

#define G fparse
/* Funciones que escriben el mensaje de error correspondiente a 'err' e incrementan err_count
o war_count, según sea el caso. Normalmente no se llama a estas funciones directamente ya que
las funciones <Tipo>___keyval y <Tipo>___keyname (Bool___keyval, Bool___keyname, etc.) ya lo hacen.*/
static inline void error_kvBool(FConfigParse *fparse, const MacObj* kv){
	ifzlike(fparse->errors.err) return;
	error_Bool_err(fparse->errors.err,&fparse->errors,KeyFile(G,kv),KeyLine(G,kv),KeyKey(G,kv),KeyStr(G,kv));
}
static inline void error_kvEntero(FConfigParse *fparse, const MacObj* kv, uint max_val, uint _unused(max_recom)){
	ifzlike(fparse->errors.err) return;
	error_Entero_err(fparse->errors.err,&fparse->errors,KeyFile(G,kv),KeyLine(G,kv),KeyKey(G,kv),KeyStr(G,kv),kv->value.val,max_val,max_recom);
}
static inline void error_kvsMedida(FConfigParse *fparse, const MacObj* kv, uint max_val, uint _unused(max_recom)){
	ifzlike(fparse->errors.err) return;
	ssint sm=kv->value.sval;
	error_Magnitud_err(fparse->errors.err,&fparse->errors,KeyFile(G,kv),KeyLine(G,kv),KeyKey(G,kv),KeyStr(G,kv),sm<0,(uint)(sm>=0? sm:-sm),max_val,max_recom,"dmm");
}
static inline void error_kvFloat(FConfigParse *fparse, const MacObj* kv, float max_val, float _unused(max_recom)){
	ifzlike(fparse->errors.err) return;
	error_Float_err(fparse->errors.err,&fparse->errors,KeyFile(G,kv),KeyLine(G,kv),KeyKey(G,kv),KeyStr(G,kv),kv->value.fl,max_val,max_recom);
}
#undef G

//Devuelve el KeyVal de la variable de nombre pc
//o bien {.macro_type=MACRO_OBJECT, .p.obj=NULL} si no existe.
//pc is '\0'-ended
static inline KeyVal find_key_0(iconst KeyVals *kvs, const char8_t *pc){
	KeyVal kv={.macro_type=MACRO_OBJECT, .p.obj=NULL};
	WhereKey *w=geth_data(hBase8WhereKey,&kvs->hash,pc);
	if(w!=NULL){
		switch(kv.macro_type=w->macro_type){
		case MACRO_OBJECT: kv.p.obj=kvs->objs.ppio+w->n; break;
		case MACRO_FUNC: kv.p.fun=kvs->funcs.ppio+w->n; break;
		//default: Let it NULL
		}
	}
	return kv;
}


/****---    Funciones que toman un keyname o MacObj y devuelven su 'val' interpretado como un cierto tipo (Bool, etc.)   ---****/

//Igual que String___keyname (vidi) pero recibiendo directamente un MacObj* en lugar del nombre de la clave
char8_t* String___keyval(MacObj *kv, FConfigParse *fparse, char8_t *s, uint n);
//Igual que Bool___keyname (vidi) pero recibiendo directamente un MacObj* en lugar del nombre de la clave
void Bool___keyval(MacObj *kv, FConfigParse *fparse);
//Igual que Entero___keyname (vidi) pero recibiendo directamente un MacObj* en lugar del nombre de la clave
void Entero___keyval(MacObj *kv, FConfigParse *fparse, uint max_val, uint max_recom);
//Igual que Medida___keyname (vidi) pero recibiendo directamente un MacObj* en lugar del nombre de la clave
void Medida___keyval(MacObj *kv, FConfigParse *fparse, uint max_val, uint max_recom);
//Igual que sMedida___keyname (vidi) pero recibiendo directamente un MacObj* en lugar del nombre de la clave
void sMedida___keyval(MacObj *kv, FConfigParse *fparse, uint max_val, uint max_recom);
//Igual que uFloat___keyname (vidi) pero recibiendo directamente un MacObj* en lugar del nombre de la clave
void uFloat___keyval(MacObj *kv, FConfigParse *fparse, float max_val, float max_recom);
//Igual que sFloat___keyname (vidi) pero recibiendo directamente un MacObj* en lugar del nombre de la clave
void sFloat___keyval(MacObj *kv, FConfigParse *fparse, float max_val, float max_recom);

/* Copia una cadena de texto guardada como valor de la clave keyname a un buffer de tamaño 'n'.
Si la cadena en kv es más larga que 'n' mostrará el mensaje de error y la cadena quedará truncada.
No cambia kv->value.type.

   s: Buffer donde se copiará la cadena de texto
   n: Tamaño del buffer
   keyname: Key/Val cuyo 'val', como string, se copiará a 's'.

Devuelve el puntero al '\0' del final de la cadena copiada. */
static inline char8_t* String___keyname(const char8_t *keyname, FConfigParse *fparse, char8_t *s, uint n){
	KeyVal kv=find_key_0(&fparse->kvs,keyname);
	if(kv.macro_type!=MACRO_OBJECT){fparse->errors.err=KEY_NOTOBJECT; return NULL;}
	if(kv.p.obj==NULL){fparse->errors.err=KEY_NOT_FOUND; return NULL;}
	return String___keyval(kv.p.obj,fparse,s,n);
}

/*Devuelve el 'val' del un MacObj como booleano.
Si el string que está guardado como 'val' de la clave aún no se ha parseado para leerlo como un
booleano, se hará, y entonces:
	El valor leído se guardará en kv->value.b
	Se asignará kv->value.type=VAL_TYPE_bool o VTYPE_erroneous_bool, según el parseo
		tenga éxito o no.
	Si se produce algún error/warning se escribirá el mensaje de error/warning.

Si tras llamar a la función fparse->errors.err es <0 el valor devuelto puede estar mal. Equivale
a kv->value.type=VTYPE_erroneous_bool. Indican que el string no se puede interpretar como
un bool.
 */
static inline bool8 Bool___keyname(const char8_t *keyname, FConfigParse *fparse){
	KeyVal kv=find_key_0(&fparse->kvs,keyname);
	if(kv.macro_type!=MACRO_OBJECT){fparse->errors.err=KEY_NOTOBJECT; return 0;}
	if(kv.p.obj==NULL){fparse->errors.err=KEY_NOT_FOUND; return 0;}
	Bool___keyval(kv.p.obj,fparse);
	return (bool8)kv.p.obj->value.val;
}

/*Devuelve el 'val' del un MacObj como entero (>=0).
Si el string que está guardado como 'val' de la clave aún no se ha parseado para leerlo como un
entero, se hará, y entonces:
	El valor leído se guardará en kv->value.val
	Se asignará kv->value.type=VAL_TYPE_entero o VTYPE_erroneous_entero, según el parseo
		tenga éxito o no.
	Si se produce algún error/warning se escribirá el mensaje de error/warning.

max_val: Valor máximo posible del tipo de dato en el que se va a almacenar el entero leído. P. e., 255 para un uint8.
max_range: Máximo valor esperable.

Si tras llamar a la función fparse->errors.err es <0 el valor devuelto puede estar mal. Equivale
a kv->value.type=VTYPE_erroneous_entero. Indican que el string no se puede interpretar como
un entero (>=0).
	Un valor > max_val provoca un error ERR_VAL_RANGE (<0). Un valor > max_recom pero <= max_val
provoca un aviso WAR_VAL_RANGE (>0).
 */
static inline uint Entero___keyname(const char8_t *keyname, FConfigParse *fparse, uint max_val, uint max_recom){
	KeyVal kv=find_key_0(&fparse->kvs,keyname);
	if(kv.macro_type!=MACRO_OBJECT){fparse->errors.err=KEY_NOTOBJECT; return 0;}
	if(kv.p.obj==NULL){fparse->errors.err=KEY_NOT_FOUND; return 0;}
	Entero___keyval(kv.p.obj,fparse,max_val,max_recom);
	return kv.p.obj->value.val;
}

/*Devuelve el 'val' del un MacObj como una medida, que ha de ser >=0. El valor se devuelve en dmm.
Si el string que está guardado como 'val' de la clave aún no se ha parseado para leerlo como una
medida, se hará, y entonces:
	El valor leído se guardará en kv->value.sval, en dmm.
	Se asignará kv->value.type=VAL_TYPE_sMedida o VTYPE_erroneous_sMedida, según el
	parseo tenga éxito o no.
	Si se produce algún error/warning se escribirá el mensaje de error/warning.

max_val: Valor máximo posible del tipo de dato en el que se va a almacenar la medida leída, o bien
			valor máximo del correspondiente tipo con signo. P. e., MAX_MEDIDAS.
max_range: Máximo valor esperable.

Si tras llamar a la función fparse->errors.err es <0 el valor devuelto puede estar mal. Equivale
a kv->value.type=VTYPE_erroneous_sMedida. Indican que el string no se puede interpretar
como una medida >=0.
	Un valor > max_val provoca un error ERR_VAL_RANGE (<0). Un valor > max_recom pero <= max_val
provoca un aviso WAR_VAL_RANGE (>0).
 */
static inline uint Medida___keyname(const char8_t *keyname, FConfigParse *fparse, uint max_val, uint max_recom){
	KeyVal kv=find_key_0(&fparse->kvs,keyname);
	if(kv.macro_type!=MACRO_OBJECT){fparse->errors.err=KEY_NOTOBJECT; return 0;}
	if(kv.p.obj==NULL){fparse->errors.err=KEY_NOT_FOUND; return 0;}
	Medida___keyval(kv.p.obj,fparse,max_val,max_recom);
	return (uint)kv.p.obj->value.sval;
}

/*Devuelve el 'val' del un MacObj como una medida. El valor se devuelve en dmm.
Si el string que está guardado como 'val' de la clave aún no se ha parseado para leerlo como una
medida, se hará, y entonces:
	El valor leído se guardará en kv->value.sval, en dmm.
	Se asignará kv->value.type=VAL_TYPE_sMedida o VTYPE_erroneous_sMedida, según el
	parseo tenga éxito o no.
	Si se produce algún error/warning se escribirá el mensaje de error/warning.

max_val: Valor máximo posible del tipo de dato en el que se va a almacenar la medida leída. P. e., MAX_MEDIDAS.
max_range: Máximo valor esperable.

Si tras llamar a la función fparse->errors.err es <0 el valor devuelto puede estar mal. Equivale
a kv->value.type=VTYPE_erroneous_sMedida. Indican que el string no se puede interpretar
como una medida.
	Un valor absoluto de la medida leída > max_val provoca un error ERR_VAL_RANGE (<0). Un valor
absoluto > max_recom pero <= max_val provoca un aviso WAR_VAL_RANGE (>0).
 */
static inline ssint sMedida___keyname(const char8_t *keyname, FConfigParse *fparse, uint max_val, uint max_recom){
	KeyVal kv=find_key_0(&fparse->kvs,keyname);
	if(kv.macro_type!=MACRO_OBJECT){fparse->errors.err=KEY_NOTOBJECT; return 0;}
	if(kv.p.obj==NULL){fparse->errors.err=KEY_NOT_FOUND; return 0;}
	sMedida___keyval(kv.p.obj,fparse,max_val,max_recom);
	return kv.p.obj->value.sval;
}

/*Devuelven el 'val' del un MacObj como float. Para la función uFloat___keyname este ha de ser >=0.
Si el string que está guardado como 'val' de la clave aún no se ha parseado para leerlo como un
entero, se hará, y entonces:
	El valor leído se guardará en kv->value.fl
	Se asignará kv->value.type=VAL_TYPE_float o VTYPE_erroneous_float, según el parseo
		tenga éxito o no.
	Si se produce algún error/warning se escribirá el mensaje de error/warning.

max_val: Valor máximo posible del tipo de dato en el que se va a almacenar el float leído. P. e., FLT_MAX.
max_range: Máximo valor esperable.

Si tras llamar a la función fparse->errors.err es <0 el valor devuelto puede estar mal. Equivale
a kv->value.type=VTYPE_erroneous_float. Indican que el string no se puede interpretar como
un float o, para uFloat___keyname, que no es >=0.
	Un valor absoluto > max_val provoca un error ERR_VAL_RANGE (<0). Un valor absoluto > max_recom
pero <= max_val provoca un aviso WAR_VAL_RANGE (>0).
 */
static inline float uFloat___keyname(const char8_t *keyname, FConfigParse *fparse, float max_val, float max_recom){
	KeyVal kv=find_key_0(&fparse->kvs,keyname);
	if(kv.macro_type!=MACRO_OBJECT){fparse->errors.err=KEY_NOTOBJECT; return 0;}
	if(kv.p.obj==NULL){fparse->errors.err=KEY_NOT_FOUND; return 0;}
	uFloat___keyval(kv.p.obj,fparse,max_val,max_recom);
	return kv.p.obj->value.fl;
}
static inline float sFloat___keyname(const char8_t *keyname, FConfigParse *fparse, float max_val, float max_recom){
	KeyVal kv=find_key_0(&fparse->kvs,keyname);
	if(kv.macro_type!=MACRO_OBJECT){fparse->errors.err=KEY_NOTOBJECT; return 0;}
	if(kv.p.obj==NULL){fparse->errors.err=KEY_NOT_FOUND; return 0;}
	sFloat___keyval(kv.p.obj,fparse,max_val,max_recom);
	return kv.p.obj->value.fl;
}

#ifdef __cplusplus
}
#endif
VISIBILITY_BLOCK_END
