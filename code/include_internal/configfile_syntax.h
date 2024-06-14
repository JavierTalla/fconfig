#undef COMMENT_CHAR
#define COMMENT_CHAR '%'

#define isopenvar(c) (c=='(' || c=='{' || c=='<')
#define isopen(c) (c=='[' || c=='(' || c=='{' || c=='<')
#define isclose(c) (c==']' || c==')' || c=='}' || c=='>')

//Characters or sequences thereof that mark the end of a key
typedef enum{ASS_NONE=0,
	ASS_Eq,			// =
	ASS_EqDef,		// :=
} Tok_Assign;

//Kind of relational operations
typedef enum{REL_NONE=0,
	REL_Eq,			// =
	REL_Neq,		// ≠
	REL_Less,		// <
	REL_Leq,		// ≤
	REL_Gt,			// >
	REL_Geq,		// ≥
} Tok_Relation;
#define RELATION_MAX REL_Geq

//"0=", etc. se toman como tokens si están separados de la palabra precedente
static const char8_t * const TokenAssignment[]={
	[ASS_NONE]=u8"",
	[ASS_Eq]=u8"=",
	[ASS_EqDef]=u8":=",
};

static char16_t const TokenRelation[]={
	[REL_NONE]='\0',
	[REL_Eq]=u'=',
	[REL_Neq]=u'≠',
	[REL_Less]=u'<',
	[REL_Leq]=u'≤',
	[REL_Gt]=u'>',
	[REL_Geq]=u'≥',
};
static char8_t const *const TokenRelationStr[]={
	[REL_NONE]=u8"",
	[REL_Eq]=u8"=",
	[REL_Neq]=u8"≠",
	[REL_Less]=u8"<",
	[REL_Leq]=u8"≤",
	[REL_Gt]=u8">",
	[REL_Geq]=u8"≥",
};
#define TOKSREL_SIZE (RELATION_MAX+1)

//El primer índice es el operador.
//El segundo índice es la relación entre los operandos:
//	0: <.  1: =  2: >
static bint const RelationSatisfied[][3]={
	[REL_NONE]={1,1,1},
	[REL_Eq]={0,1,0},
	[REL_Neq]={1,0,1},
	[REL_Less]={1,0,0},
	[REL_Leq]={1,1,0},
	[REL_Gt]={0,0,1},
	[REL_Geq]={0,1,1},
};

//op: El operador entre los elementos a comparar: =, !=, <, etc,
//resultado: El resultado de una comparación entre los operandos:
//	-1: <   0: =   1: >
#define isRelationSatisfied(op,resultado) RelationSatisfied[op][1+(resultado)]

//Keywords at beginning of line
typedef enum{
	KW_NONE=0, //No es k.w.
	//
	KW_logging,
	KW_logonline,
	KW_loggingall,
	KW_loggingnone,
	KW_loggingdefault,
	KW_logerrors,
	KW_logfiles,
	KW_logcommands,
	KW_logassigns,
	KW_logbranches,
	KW_logvars,
	KW_logmacros,
	KW_formatmsg,
	//
	KW_ModoOverride,
	KW_remove,
	KW_ParseIfNotDef,
	KW_ParseIfEmpty,
	KW_copyroot,
	KW_cleanspaces,
	KW_type,
	//
	KW_show,
	KW_showall,
	KW_dump,
	KW_echo,
	KW_Echo,
	KW_error,
	KW_warning,
	KW_FILE,
	KW_LINE,
	//
	KW_include,
	KW_jump,
	KW_stop,
	KW_If,
	KW_Ifdef,
	KW_Ifndef,
	KW_Ifempty,
	KW_Else,
	KW_Elif,
	KW_Endif,
	KW_AtEnd,
	KW_BeginAtEnd,
	KW_EndAtEnd
} KeyWord;

#define KeyWord_Min KW_logging
#define KeyWord_logMin KW_logging
#define KeyWord_logTope (KW_logmacros+1)
#define KeyWord_Max KW_EndAtEnd
#define KeyWord_Tope (KW_EndAtEnd+1)

#define KeyWord_LogcasesMin KW_logfiles
#define KeyWord_LogcasesTope (KW_formatmsg+1)

#define KWFLAG_Somethig 1 //Si tras la palabra tiene que haber algo más
#define KWFLAG_Assignment 2 //Si lo que sigue se interpreta como asignación
#define KWFLAG_Texto 4 //Si lo que sigue a la palabra clave se interpreta siempre como texto; e.d., un =, etc., no indica una asignación
#define KWFLAG_AssTex 6
/* Si para una cierta keyword
	KWFLAG_Assignment=true, KWFLAG_Texto=false: Tiene que seguir obligatoriamente una asignación
	KWFLAG_Assignment=flase, KWFLAG_Texto=false: Puede seguir una asignación. buffer.pc queda apuntando a
la palabra tras la asignación, si la hay; si no hay, a la palabra que sigue a la clave.
	KWFLAG_Assignment=flase KWFLAG_Texto=true: Lo que sigue nunca se interpreta como asignación.
	KWFLAG_Assignment=true, KWFLAG_Texto=true: No puede seguir una asignación, pero se mira si lo que sigue
parece una ignación. Si es así constituye un error.
*/

#define DEFKEmpty(k) [KW_##k]={u8"" #k, 0}
#define DEFKVal(k) [KW_##k]={u8"" #k, KWFLAG_Somethig|KWFLAG_Assignment}
#define DEFKBareVal(k) [KW_##k]={u8"" #k, KWFLAG_Somethig}
#define DEFKName(k) [KW_##k]={u8"" #k, KWFLAG_Somethig|KWFLAG_AssTex}
#define DEFKPureText(k) [KW_##k]={u8"" #k, KWFLAG_Somethig|KWFLAG_Texto}

static const struct KeyWordProps{
	const char8_t * const name;
	uint flags;
} KeyWords[KeyWord_Max+1]={
	[KW_NONE]={u8"",KWFLAG_Assignment}, //No tiene KWFLAG_Somethig. No es necesario que siga nada
	DEFKName(logging),
	DEFKEmpty(logonline),
	DEFKEmpty(loggingall),
	DEFKEmpty(loggingnone),
	DEFKEmpty(loggingdefault),
	DEFKVal(logfiles),
	DEFKVal(logerrors),
	DEFKVal(logcommands),
	DEFKVal(logassigns),
	DEFKVal(logbranches),
	DEFKVal(logvars),
	DEFKVal(logmacros),
	DEFKBareVal(formatmsg),
	DEFKBareVal(ModoOverride),
	DEFKName(remove),
	DEFKName(ParseIfNotDef),
	DEFKName(ParseIfEmpty),
	DEFKName(copyroot),
	DEFKName(cleanspaces),
	DEFKName(type),
	DEFKName(show),
	DEFKEmpty(showall),
	DEFKEmpty(dump),
	DEFKPureText(echo),
	DEFKPureText(Echo),
	DEFKPureText(error),
	DEFKPureText(warning),
	DEFKPureText(FILE),
	DEFKVal(LINE),
	DEFKPureText(include),
	DEFKPureText(jump),
	DEFKEmpty(stop),
	DEFKName(If),
	DEFKName(Ifdef),
	DEFKName(Ifndef),
	DEFKName(Ifempty),
	DEFKEmpty(Else),
	DEFKName(Elif),
	DEFKEmpty(Endif),
	[KW_AtEnd]={u8"AtEnd",KWFLAG_Texto}, //nothing need follow. If follows, as text.
	DEFKEmpty(BeginAtEnd),
	DEFKEmpty(EndAtEnd),
};

#undef DEFKempty
#undef DEFKVal
#undef DEFKBareVal
#undef DEFKName
#undef DEFKPureText

//Tipos de if-branching (empezar, cambiar de caso, terminar)
#define IFTYPE_NONE 0
#define IFTYPE_IF 1
#define IFTYPE_ELSE 3
#define IFTYPE_ENDIF 4

//Para distinguir entre else y el/los elif mírese simplemente si k.-w. es KW_Else
static const umint Iftype[KeyWord_Max+1]={
	[KW_If]=IFTYPE_IF,
	[KW_Ifdef]=IFTYPE_IF,
	[KW_Ifndef]=IFTYPE_IF,
	[KW_Ifempty]=IFTYPE_IF,
	[KW_Elif]=IFTYPE_ELSE,
	[KW_Else]=IFTYPE_ELSE,
	[KW_Endif]=IFTYPE_ENDIF
};

//Pareja KeyWord + Assignment
typedef struct{
	KeyWord kw;
	Tok_Assign ass;
} KwAss;

#define IFState_True 0 //We are at the true branch. Corresponds with m=lev below.
#define IFState_Ready 1 //We are skipping through a false branch. The true branch has already been found.
#define IFState_Waiting 2 //We are skipping through a false branch. The true branch has not yet been reached.

typedef struct{
	uint8m lev; //Level of the outermost false branch in which we are; otherwise, current innermost level (true). lev<=m
	uint8m m; //current if level. m>=lev. If m=lev and state=IFState_True we are processing the code; otherwise we are skipping the code.
	umint state; //skip mode del nivel lev. One of avobe
} IfState;

//Estado global del intérprete
typedef struct{
	uint nfile; //Current file being read
	uint8m file_lev; //Level of inntermost file being read. The outermost is 1.
	uint atend_nfile; //File which is current at atend
	umint override; //Modo de override: 0: override; 1: no override.
} InterpreterState;

//Estructura que describe el estado específico del fichero que se está leyendo
//Si hay una cadena de ficheros abiertos cada uno tiene su LocalState.
typedef struct{
	IfState ifS;
} LocalState;

typedef struct{
	uint8m fich,
		commands,
		branches,
		assigns,
		vars,
		macros;
	uint8m formatmsg;
} strLog;

//Límites

#define BRACKETS_MAX_N 10000

//Códigos de error
#define VARS_SYNTAX 1
#define VARS_RUNAWAY 2
#define EXPANSION_RECURSIVE 3
#define VAR_MUST_EXIST 4
#define VARS_SEMANTICS 5
#define VAR_INVALID_NAME 6
#define VAR_MISSING_ARGLIST 7
#define VAR_MUSTBE_FUNC 8
#define EXP_IMMEDIATE_RECURSIVE 100
#define PARSEIF_VarUndefined 20
