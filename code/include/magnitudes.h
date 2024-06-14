//Tipo de dato según las unidades que le pueden seguir:
enum TipoMagnitud{Magnitud_Tabla=1, Magnitud_TierraZ, Magnitud_TierraPeq, Magnitud_TierraGrande, Magnitud_Tiempo};

typedef uint8m Unidad;

//UNI_WRONG será mucas veces la consecuencia de que el usuario se ha equivocado al escribir o la ha omitido y se ha leído la palabra siguiente
#define UNI_WRONG 0x7F //El texto indicado no es el de ninguna unidad conocida
#define UNI_INVALID 0x7E //No es ninguna unidad válida para el dato a que acompaña
#define UNI_ABSENT 0
//
#define UNI_DMM 1 //Décimas de milímetro
#define UNI_MM 2
#define UNI_CM 3
#define UNI_M 5
//
#define UNI_ARC_DTER 6 //Décimas de tercero
#define UNI_ARC_TER 7 //Terceros de arco
#define UNI_ARC_SEG 8 //Segundos de arco
//
#define UNI_KM 9
#define UNI_ARC_GRADOS 10

// Unidades de tiempo
#define UNI_T_MSEC 1
#define UNI_T_SEC 2
#define UNI_T_MIN 3

#define MAX_UNI_LONG 10
static const char8_t * const unidades_long[MAX_UNI_LONG+1]={u8"(nada)",u8"dmm",u8"mm",u8"cm",u8"(nada)",u8"m",u8"dter",u8"\"'",u8"\"",u8"km",u8"º"};

//res
#define MAGRES_OK 0		//Se almacenó el valor indicado de manera exacta.
#define MAGRES_ROUND 1 //Se produjo error de redondeo (se ignoraron decimales)
									//o bien es val=MAX_MAGNI_VAL

/*Un dato con sus unidades. Se genera al leer un dato del fichero, e inmediatamente
se almacenará el valor en las unidades correspondientes al campo en cuestión.
Por ejemplo, una Magnitud_Tabla se almacena en dmm. */
typedef struct{
	unsigned val: 24;
	unsigned uni: 7;
	unsigned res: 1; //Si es !=0 no se pudo almacenar el valor exacto.
} LongitudLeída; //Las unidades pueden ser cualesquiera

typedef LongitudLeída
	LongitudTabla,			//Las unidades pueden ir de dmm a m
	LongitudTierraPeq,	//Las unidades pueden ser cm, m, arc_dter o arc_seg
	LongitudTierraGrande, //uni puede ser m, Km, arc_seg o arc_grados.
	Tiempo;

typedef struct{
	int err; //0: todo bien. ERR_VAL_EMPTY. ERR_VAL_NOBOOLEAN. In this case val may be wrong or unasigned
	bool8 val;
} BoolValidado;

typedef struct{
	int err; //0: todo bien. >0: warning code. <0: error. In this case val may be wrong or unasigned
	uint val;
} MagnitudValidada, EnteroValidado;

typedef struct{
	int err; //0: todo bien. >0: warning code. <0: error. In this case val may be wrong or unasigned
	ssint val;
} sMagnitudValidada;

typedef struct{
	int err; //0: todo bien. >0: warning code. <0: error. In this case val may be wrong or unasigned
	float val;
} FloatValidado;

#define MAX_MAGNI_VAL ((1<<24U)-1)
#define MAX_MAGNI_VAL_String "16.000.000"

#define ERR_VAL_EMPTY -21
#define ERR_VAL_MISSFORMAT -20
#define ERR_VAL_NONUMBER -20
#define ERR_VAL_NOBOOLEAN -20
#define ERR_VAL_NUMHUGE -19	//El número indicado es tan grande no se puede leer (>= 1<<24)
#define ERR_UNI_ABSENT -10
#define ERR_UNI_WRONG -9
#define ERR_VAL_HUGE -8		//Es tan grande que no se puede almacenar, en las unidades en las que se quiere almacenar (>Я)
#define ERR_VAL_NEG -7		//Aquí no se permiten valores negativos. (Este error nunca salta en la lectura. Se asigna si acaso después)
#define ERR_VAL_RANGE -6 //Está fuera del rango de valores permitido en el momento de la lectura.
									  //Por ejemplo, >255 si tiene que ir en 8 bits. Más adelante puede limitarse más
#define WAR_VAL_RES 1		//Ha sido necesario redondear el dato, ignorando los últimos dígitos escritos.
#define WAR_VAL_RANGE 2 //Está fuera del rango de valores esperado.

/*bool8 lee_Bool(const char8_t **pc);
static inline uint lee_Entero(const char8_t **pc){return uint___str8(*pc,pc);}
LongitudLeída lee_Magnitud_no_decimales(iconst char8_t **pc, enum TipoMagnitud typeM);
LongitudLeída lee_Magnitud_decimales(iconst char8_t **pc, enum TipoMagnitud typeM);
*/
