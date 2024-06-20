/* Funciones lee_valida_<Dato>

Parsea la cadena apuntada por *pc. El dato puede estar terminado por cualquier blanco: ' ', '\t' o '\n'.
*pc se avanza al final de la unidad, salvo que se produzca algún error en cuyo caso queda donde el error.
Si tras la función d.err<0 (siendo d lo devuelto por la función) el dato almacenado puede ser erróneo.
    La función recibe un parámetro 'pc' y, dependiendo del tipo de Dato a leer algunos más:

pc:				*pc es el puntero a lo que hay que leer.
max_val:		Valor máximo permitido. Siempre hay uno; por ejemplo, el máximo almacenable en el tipo de dato de destino.
max_recom:	Valor máximo recomendado o esperado. Si se supera se pondrá d.err a WAR_VAL_RANGE.
uni:			Unidades en las que se guardará el resultado, en d.val


Funciones lee_valida_<Magnitud>

Si <Dato> es una Magnitud tiene que haber un número seguido de unas unidades válidas.
Devuelve en .val el dato pasado a las unidades 'uni'. Las unidades que sigan al número tienen que ser
las permitidas para el tipo de magnitud que se va a leer, que puede ser:

Magnitud_Tabla    Magnitud_TierraZ    Magnitud_TierraPeq
Magnitud_TierraGrande    Magnitud_Tiempo

'uni' indica las unidades en las que se quiere almacenar el dato. Por ejemplo, si son UNI_CM y el dato
leído es 1,20 m, se devolverá 120. Los valores válidos dependen también del tipo de magnitud que se lee.

Para cada Magnitud hay a su vez dos o tres variantes, que se ejemplifican con LongitudTabla:

lee_valida_LongitudTabla:
	Si **pc no está entre '0' y '9' se hará d.err=ERR_VAL_NONUMBER y la función devuelve inmediatamente.

lee_valida_uLongitudTabla:
	Si **pc no está entre '0' y '9' se hará d.err=ERR_VAL_NONUMBER, salvo porque si **pc=='-' y
	*(pc[1]) está entre '0' y '9' se hará d.err=ERR_VAL_NEG. En ambos casos la función devuelve
	inmediatamente.

lee_valida_sLongitudTabla:
	Sea c= **pc=='-'? **pc : *(pc[1]). Si c no está entre '0' y '9' se hará d.err=ERR_VAL_NONUMBER y
	la función devuelve inmediatamente.
*/

extern BoolValidado lee_valida_Bool(iconst char8_t **pc);
extern EnteroValidado lee_valida_Entero(iconst char8_t **pc, uint max_val, uint max_recom);
extern FloatValidado lee_valida_uFloat(iconst char8_t **pc, float max_val, float max_recom);
extern FloatValidado lee_valida_sFloat(iconst char8_t **pc, float max_val, float max_recom);

/*Las unidades en las que esté expresado el dato tienen que ser dmm, mm, cm o cm.
'uni' es las unidades en las que se almacenará en d.val. Puede ser UNI_DMM, UNI_MM o UNI_CM*/
extern MagnitudValidada lee_valida_LongitudTabla(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni);
extern MagnitudValidada lee_valida_uLongitudTabla(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni);
extern sMagnitudValidada lee_valida_sLongitudTabla(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni);

/* Las unidades en las que esté expresado el dato tienen que ser m o cm.
'uni' es las unidades en las que se almacenará en d.val. Puede ser UNI_M o UNI_CM */
extern MagnitudValidada lee_valida_LongitudTierraZ(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni);
extern MagnitudValidada lee_valida_uLongitudTierraZ(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni);
extern sMagnitudValidada lee_valida_sLongitudTierraZ(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni);

/* Las unidades en las que esté expresado el dato pueden ser m, cm, segundos de arco, terceros de arco o décimas de tercero.
'uni' es las unidades en las que se almacenará en d.val. Puede ser UNI_M o UNI_CM */
extern MagnitudValidada lee_valida_LongitudTierraPeq(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni);
extern MagnitudValidada lee_valida_uLongitudTierraPeq(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni);
extern sMagnitudValidada lee_valida_sLongitudTierraPeq(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni);

/* Las unidades en las que esté expresado el dato pueden ser m, km, grados o segundos de arco.
'uni' es las unidades en las que se almacenará en d.val. Puede ser UNI_KM o UNI_M */
extern MagnitudValidada lee_valida_LongitudTierraGra(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni);
extern MagnitudValidada lee_valida_uLongitudTierraGra(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni);

/* Las unidades en las que esté expresado el dato pueden ser UNI_T_MIN o UNI_T_SEC.
'uni' es las unidades en las que se almacenará en d.val. Puede ser UNI_T_MSEC o UNI_T_SEC */
extern MagnitudValidada lee_valida_Tiempo(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni);
extern MagnitudValidada lee_valida_uTiempo(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni);


//Igual que las funciones error_<Dato> sin la terminación _err, pero omitiendo la optimización para cuando err=0.
extern void error_Bool_err(int err, ErrorOpts *errors, const char8_t *file, uint line, const char8_t* key, const char8_t *str);
extern void error_Entero_err(int err, ErrorOpts *errors, const char8_t *file, uint line, const char8_t* key, const char8_t *str, uint _unused(val), uint max_val, uint _unused(max_recom));
extern void error_Magnitud_err(int err, ErrorOpts *errors, const char8_t *file, uint line, const char8_t* key, const char8_t *str, bint bneg, uint val, uint max_val, uint _unused(max_recom), const char8_t *uni);
extern void error_Float_err(int err, ErrorOpts *errors, const char8_t *file, uint line, const char8_t* key, const char8_t *str, float _unused(val), float max_val, float _unused(max_recom));

/* Funciones que escriben el mensaje de error correspondiente a 'err' e incrementan err_count o war_count,
    según sea el caso.

err:			Contiene el error que se produjo al intentar leer un valor del tipo indicado en el nombre de la función.
errors:		Para el mensaje de error e incrementar err_count o war_count.
file:			Nombre del fichero en que se leyó la clave. Puede ser NULL.
key:			El nombre de la clave.
str:			Lo que se leyó del fichero tal cual.
   Los siguientes parámetros se emplean tal vez para mostrar el mensaje de error y no tienen mucha importancia
bneg, val:	Si err>=0, el valor es +val o -val, según sea bneg false o true.
max_val:		Máximo valor permitido
max_recom:	Máximo valor esperado.
uni:			Texto con las unidades en las que está el valor.

Si err= 0 la función no hace nada, y al ser inline no habrá ninguna llamada a función.
si err!=0 incrementa err_count o war_count. En ese caso sí se llama a la función que hace el trabajo.
*/
static inline void error_Bool(int err, ErrorOpts *errors, const char8_t *file, uint line, const char8_t* key, const char8_t *str){
	ifzlike(err) return;
	error_Bool_err(err,errors,file,line,key,str);
}
static inline void error_Entero(int err, ErrorOpts *errors, const char8_t *file, uint line, const char8_t* key, const char8_t *str, uint _unused(val), uint max_val, uint _unused(max_recom)){
	ifzlike(err) return;
	error_Entero_err(err,errors,file,line,key,str,val,max_val,max_recom);
}
static inline void error_Magnitud(int err, ErrorOpts *errors, const char8_t *file, uint line, const char8_t* key, const char8_t *str, uint val, uint max_val, uint _unused(max_recom), const char8_t *uni){
	ifzlike(err) return;
	error_Magnitud_err(err,errors,file,line,key,str,false,val,max_val,max_recom,uni);
}
static inline void error_sMagnitud(int err, ErrorOpts *errors, const char8_t *file, uint line, const char8_t* key, const char8_t *str, ssint sval, uint max_val, uint _unused(max_recom), const char8_t *uni){
	ifzlike(err) return;
	error_Magnitud_err(err,errors,file,line,key,str,sval<0,(uint)(sval>=0? sval:-sval),max_val,max_recom,uni);
}
static inline void error_Float(int err, ErrorOpts *errors, const char8_t *file, uint line, const char8_t* key, const char8_t *str, float _unused(val), float max_val, float _unused(max_recom)){
	ifzlike(err) return;
	error_Float_err(err,errors,file,line,key,str,val,max_val,max_recom);
}


/* Funciones que parsean un dato y lo guardan en un ObjectValue */

typedef int (*Parse_Value)(ObjectValue *dato, const char8_t *pc);

//Devuelven el error al parsear. (0, si no hay)
extern int parse_Bool(ObjectValue *dato, const char8_t *pc);
extern int parse_Entero(ObjectValue *dato, const char8_t *pc);
extern int parse_Medida(ObjectValue *dato, const char8_t *pc);
extern int parse_uFloat(ObjectValue *dato, const char8_t *pc);
extern int parse_unknown(ObjectValue *dato, const char8_t *pc);


/* Funciones que parsean y validan un dato y ecriben el error en su caso, y devuelven un <Tipo>Validado */

/*Parsea la cadena pc, que puede contener más datos que lo que se va a leer, pero en cualquier caso tiene que terminar en '\0'.
Si se produce un error actualiza errors y escribe el mensaje de error, mostrando fieldname y, si no es NULL, filename.
filename puede ser NULL*/
static inline BoolValidado consume_Bool(iconst char8_t **pc, ErrorOpts *errors, const char8_t* fieldname, const char8_t *filename, uint line){
	const char8_t * const pstr=*pc;
	BoolValidado d=lee_valida_Bool(pc);
	error_Bool(d.err,errors,filename,line,fieldname,pstr);
	return d;
}

/*Parsea la cadena pc, que puede contener más datos que lo que se va a leer, pero en cualquier caso tiene que terminar en '\0',
salvo que estemos seguros de que no se va a producir un error; e. d., que la cadena apunta a un número terminado por un blanco
y el número es <= que max_recom (que ha de ser <= max_val).
Si se produce un error actualiza errors y escribe el mensaje de error, mostrando fieldname y, si no es NULL, filename.
filename puede ser NULL*/
static inline EnteroValidado consume_Entero(iconst char8_t **pc, uint max_val, uint max_recom, ErrorOpts *errors, const char8_t* fieldname, const char8_t *filename, uint line){
	const char8_t * const pstr=*pc;
	EnteroValidado d=lee_valida_Entero(pc,max_val,max_recom);
	error_Entero(d.err,errors,filename,line,fieldname,pstr,d.val,max_val,max_recom);
	return d;
}

/*Parsea la cadena pc, que puede contener más datos que lo que se va a leer, pero en cualquier caso tiene que terminar en '\0'.
Las unidades presentes en la cadena pc pueden ser m, cm, mm o dmm. El dato se guarda como un entero en dmm.
Si se produce un error actualiza errors y escribe el mensaje de error, mostrando fieldname y, si no es NULL, filename.
filename puede ser NULL*/
static inline MagnitudValidada consume_Medida_dmm(iconst char8_t **pc, uint max_val, uint max_recom, ErrorOpts *errors, const char8_t* fieldname, const char8_t *filename, uint line){
	const char8_t * const pstr=*pc;
	MagnitudValidada d=lee_valida_uLongitudTabla(pc,max_val,max_recom,UNI_DMM);
	error_Magnitud(d.err,errors,filename,line,fieldname,pstr,d.val,max_val,max_recom,u8"dmm");
	errors->err=d.err;
	return d;
}

/*Parsea la cadena pc, que puede contener más datos que lo que se va a leer, pero en cualquier caso tiene que terminar en '\0'.
Si se produce un error actualiza errors y escribe el mensaje de error, mostrando fieldname y, si no es NULL, filename.
filename puede ser NULL*/
static inline FloatValidado consume_uFloat(iconst char8_t **pc, float max_val, float max_recom, ErrorOpts *errors, const char8_t* fieldname, const char8_t *filename, uint line){
	const char8_t * const pstr=*pc;
	FloatValidado d=lee_valida_uFloat(pc,max_val,max_recom);
	error_Float(d.err,errors,filename,line,fieldname,pstr,d.val,max_val,max_recom);
	return d;
}

/*Parsea la cadena pc, que puede contener más datos que lo que se va a leer, pero en cualquier caso tiene que terminar en '\0'.
Si se produce un error actualiza errors y escribe el mensaje de error, mostrando fieldname y, si no es NULL, filename.
filename puede ser NULL*/
static inline FloatValidado consume_sFloat(iconst char8_t **pc, float max_val, float max_recom, ErrorOpts *errors, const char8_t* fieldname, const char8_t *filename, uint line){
	const char8_t * const pstr=*pc;
	FloatValidado d=lee_valida_sFloat(pc,max_val,max_recom);
	error_Float(d.err,errors,filename,line,fieldname,pstr,d.val,max_val,max_recom);
	return d;
}

/*Parsea la cadena pc, que puede contener más datos que lo que se va a leer, pero en cualquier caso tiene que terminar en '\0'.
Las unidades presentes en la cadena pc pueden ser m, cm, mm o dmm. El dato se guarda como un entero en dmm.
Si se produce un error actualiza errors y escribe el mensaje de error, mostrando fieldname y, si no es NULL, filename.
filename puede ser NULL*/
static inline sMagnitudValidada consume_sMedida_dmm(iconst char8_t **pc, uint max_val, uint max_recom, ErrorOpts *errors, const char8_t* fieldname, const char8_t *filename, uint line){
	const char8_t * const pstr=*pc;
	sMagnitudValidada d=lee_valida_sLongitudTabla(pc,max_val,max_recom,UNI_DMM);
	error_sMagnitud(d.err,errors,filename,line,fieldname,pstr,d.val,max_val,max_recom,u8"dmm");
	errors->err=d.err;
	return d;
}

/*Parsea la cadena pc, que puede contener más datos que lo que se va a leer, pero en cualquier caso tiene que terminar en '\0'.
Las unidades presentes en la cadena pc pueden ser m, cm, mm o dmm. El dato se guarda como un entero en cm.
Si se produce un error actualiza errors y escribe el mensaje de error, mostrando fieldname y, si no es NULL, filename.
filename puede ser NULL*/
static inline MagnitudValidada consume_Medida_cm(iconst char8_t **pc, uint max_val, uint max_recom, ErrorOpts *errors, const char8_t* fieldname, const char8_t *filename, uint line){
	const char8_t * const pstr=*pc;
	MagnitudValidada d=lee_valida_uLongitudTabla(pc,max_val,max_recom,UNI_CM);
	error_Magnitud(d.err,errors,filename,line,fieldname,pstr,d.val,max_val,max_recom,u8"cm");
	errors->err=d.err;
	return d;
}

/*Parsea la cadena pc, que puede contener más datos que lo que se va a leer, pero en cualquier caso tiene que terminar en '\0'.
Las unidades presentes en la cadena pc pueden ser m o cm. El dato se guarda como un entero en m.
Si se produce un error actualiza errors y escribe el mensaje de error, mostrando fieldname y, si no es NULL, filename.
filename puede ser NULL*/
static inline sMagnitudValidada consume_sTierraZ_m(iconst char8_t **pc, uint max_val, uint max_recom, ErrorOpts *errors, const char8_t* fieldname, const char8_t *filename, uint line){
	const char8_t * const pstr=*pc;
	sMagnitudValidada d=lee_valida_sLongitudTierraZ(pc,max_val,max_recom,UNI_M);
	error_Magnitud(d.err,errors,filename,line,fieldname,pstr,d.val,max_val,max_recom,u8"m");
	errors->err=d.err;
	return d;
}

/*Parsea la cadena pc, que puede contener más datos que lo que se va a leer, pero en cualquier caso tiene que terminar en '\0'.
Las unidades presentes en la cadena pc pueden ser m o cm. El dato se guarda como un entero en m.
Si se produce un error actualiza errors y escribe el mensaje de error, mostrando fieldname y, si no es NULL, filename.
filename puede ser NULL*/
static inline MagnitudValidada consume_TierraZ_m(iconst char8_t **pc, uint max_val, uint max_recom, ErrorOpts *errors, const char8_t* fieldname, const char8_t *filename, uint line){
	const char8_t * const pstr=*pc;
	MagnitudValidada d=lee_valida_uLongitudTierraZ(pc,max_val,max_recom,UNI_M);
	error_Magnitud(d.err,errors,filename,line,fieldname,pstr,d.val,max_val,max_recom,u8"m");
	errors->err=d.err;
	return d;
}

/*Parsea la cadena pc, que puede contener más datos que lo que se va a leer, pero en cualquier caso tiene que terminar en '\0'.
Las unidades presentes en la cadena pc pueden ser m, cm, ", "' (o '") o dter . El dato se guarda como un entero en cm.
Si se produce un error actualiza errors y escribe el mensaje de error, mostrando fieldname y, si no es NULL, filename.
filename puede ser NULL*/
static inline MagnitudValidada consume_TierraPeq_cm(iconst char8_t **pc, uint max_val, uint max_recom, ErrorOpts *errors, const char8_t* fieldname, const char8_t *filename, uint line){
	const char8_t * const pstr=*pc;
	MagnitudValidada d=lee_valida_uLongitudTierraPeq(pc,max_val,max_recom,UNI_CM);
	error_Magnitud(d.err,errors,filename,line,fieldname,pstr,d.val,max_val,max_recom,u8"cm");
	errors->err=d.err;
	return d;
}

/*Parsea la cadena pc, que puede contener más datos que lo que se va a leer, pero en cualquier caso tiene que terminar en '\0'.
Las unidades presentes en la cadena pc pueden ser m, cm, ", "' (o '") o dter . El dato se guarda como un entero en metros.
Si se produce un error actualiza errors y escribe el mensaje de error, mostrando fieldname y, si no es NULL, filename.
filename puede ser NULL*/
static inline MagnitudValidada consume_TierraPeq_m(iconst char8_t **pc, uint max_val, uint max_recom, ErrorOpts *errors, const char8_t* fieldname, const char8_t *filename, uint line){
	const char8_t * const pstr=*pc;
	MagnitudValidada d=lee_valida_uLongitudTierraPeq(pc,max_val,max_recom,UNI_M);
	error_Magnitud(d.err,errors,filename,line,fieldname,pstr,d.val,max_val,max_recom,u8"m");
	errors->err=d.err;
	return d;
}

/*Parsea la cadena pc, que puede contener más datos que lo que se va a leer, pero en cualquier caso tiene que terminar en '\0'.
Las unidades presentes en la cadena pc pueden ser m, km, Km, ° o ". El dato se guarda como un entero en metros.
Si se produce un error actualiza errors y escribe el mensaje de error, mostrando fieldname y, si no es NULL, filename.
filename puede ser NULL*/
static inline MagnitudValidada consume_TierraGra_m(iconst char8_t **pc, uint max_val, uint max_recom, ErrorOpts *errors, const char8_t* fieldname, const char8_t *filename, uint line){
	const char8_t * const pstr=*pc;
	MagnitudValidada d=lee_valida_uLongitudTierraGra(pc,max_val,max_recom,UNI_M);
	error_Magnitud(d.err,errors,filename,line,fieldname,pstr,d.val,max_val,max_recom,u8"m");
	errors->err=d.err;
	return d;
}

/*Parsea la cadena pc, que puede contener más datos que lo que se va a leer, pero en cualquier caso tiene que terminar en '\0'.
Si se produce un error actualiza errors y escribe el mensaje de error, mostrando fieldname y, si no es NULL, filename.
filename puede ser NULL*/
static inline MagnitudValidada consume_Tiempo_s(iconst char8_t **pc, uint max_val, uint max_recom, ErrorOpts *errors, const char8_t* fieldname, const char8_t *filename, uint line){
	const char8_t * const pstr=*pc;
	MagnitudValidada d=lee_valida_uTiempo(pc,max_val,max_recom,UNI_T_SEC);
	error_Magnitud(d.err,errors,filename,line,fieldname,pstr,d.val,max_val,max_recom,u8"s");
	errors->err=d.err;
	return d;
}

/* Funciones que parsean un dato, lo guardan en un ObjectValue y escriben el error, en su caso */

typedef void (*Parse_error_Value)(ObjectValue *dato, const char8_t *pc, ErrorOpts *errors, const char8_t *fieldname, const char8_t *filename, uint line);

//Escribe el error o aviso, si lo hay. filename puede ser NULL. Para más detalles, v. consume_Bool.
static inline void parse_error_Bool(ObjectValue *dato, const char8_t *pc, ErrorOpts *errors, const char8_t *fieldname, const char8_t *filename, uint line){
	BoolValidado d=consume_Bool(&pc,errors,fieldname,filename,line);
	if(d.err>=0) dato->b=d.val, dato->type=VAL_TYPE_bool;
	else dato->type=VTYPE_erroneous_bool;
}

//Escribe el error o aviso, si lo hay. filename puede ser NULL. Para más detalles, v. consume_Entero.
//No pone ningún límite para max_val y max_recom.
static inline void parse_error_Entero(ObjectValue *dato, const char8_t *pc, ErrorOpts *errors, const char8_t *fieldname, const char8_t *filename, uint line){
	EnteroValidado d=consume_Entero(&pc,Я,Я,errors,fieldname,filename,line);
	if(d.err>=0) dato->val=d.val, dato->type=VAL_TYPE_uint;
	else dato->type=VTYPE_erroneous_uint;
}

//Máximo valor permitido en el fichero para cualquier parámetro que sea una medida larga
#define MAX_MEDIDA_LARGA 100000000 //10.000 m (!)

//Escribe el error o aviso, si lo hay. La medida se almacenará en dmm. filename puede ser NULL. Para más detalles, v. consume_sMedida_dmm.
//Establece como límite para max_val y max_recom el valor MAX_MEDIDA_LARGA (10 metros).
static inline void parse_error_sMedida(ObjectValue *dato, const char8_t *pc, ErrorOpts *errors, const char8_t *fieldname, const char8_t *filename, uint line){
	sMagnitudValidada d=consume_sMedida_dmm(&pc,MAX_MEDIDA_LARGA,MAX_MEDIDA_LARGA,errors,fieldname,filename,line);
	if(d.err>=0) dato->sval=d.val, dato->type=VAL_TYPE_sMedida;
	else dato->type=VTYPE_erroneous_sMedida;
}

//Escribe el error o aviso, si lo hay. filename puede ser NULL. Para más detalles, v. consume_sFloat.
//No pone ningún límite para max_val y max_recom.
static inline void parse_error_Float(ObjectValue *dato, const char8_t *pc, ErrorOpts *errors, const char8_t *fieldname, const char8_t *filename, uint line){
	FloatValidado d=consume_sFloat(&pc,FLT_MAX,FLT_MAX,errors,fieldname,filename,line);
	if(d.err>=0) dato->fl=d.val, dato->type=VAL_TYPE_float;
	else dato->type=VTYPE_erroneous_float;
}
