/* Escribir al fichero de log */
#include "../include/fconfig_log.h"
#undef GET_LINE

//LERR y LWAR han de coincidir con los valores de FConfigParse_ErrOutput
#define iferrwar(errors,level) if((errors)->err_war>=level)

static void printerr_many_strings(const char8_t *s, ...){
	va_list ap;
	va_start(ap,s);
	while(s!=NULL){
		fputs((CRP)s,stderr);
		s=va_arg(ap,const char8_t*);
	}
	va_end(ap);
}

void writef_Cfile(FILE *const file, const char8_t *str, ...){
	va_list ap;
	va_start(ap, str);
	const char8_t *ps=str;
	while(*ps!='\0'){
		while(*ps!='\0' && *ps!='%'){putc(*ps,file); ps++;}
		if(*ps=='\0') break;

		ps++;
		char8_t t=*ps++;
		if(t=='s'){
			const char8_t *s=va_arg(ap,const char8_t*);
			fputs((CRP)s,file);
		}else if(t=='S'){
			t=*ps;
			iflike(*ps!='\0') ps++;
			const char8_t *s=va_arg(ap,const char8_t*);
			switch(t){
				char8_t c;
			case 's': c=' '; goto one;
			case 't': c='\t'; goto one;
			case 'n': c='\n';
			one: while(*s!=c) putc(*s,file), s++; break;
			case 'S': while(*s!=' ' && *s!='\0') putc(*s,file), s++; break;
			case 'T': while(*s!=' ' && *s!='\t') putc(*s,file), s++; break;
			case 'b': while(isnot_stn(*s)) putc(*s,file), s++; break;
			case 'B': while(isnot_stn0(*s)) putc(*s,file), s++; break;
			}
		}else switch(t){
			case 'w': va_arg(ap,const char16_t*); break; //const char16_t *s=va_arg(ap,const char16_t*); fprintf(filie,"%ls",s); break;
			case 'c': putc(va_arg(ap,int),file); break;
			case 'u': fprintf(file,"%u", va_arg(ap,unsigned int)); break;
			case 'x': fprintf(file,"%x", va_arg(ap,unsigned int)); break;
			case 'i': fprintf(file,"%i", va_arg(ap,int)); break;
			case 'f': fprintf(file,"%f", va_arg(ap,double)); break;
			case '%': default: putc(*ps,file);	//Any other character prints itself
		}
	}
	va_end(ap);
}

//Añade un '\n al final
static void log_valstr_0(Bufferto8 *blog, bint on, const char8_t *val){
	const char8_t *val2=val;
	if(blog!=NULL){ while(*val!='\0'){
		char8_t c=*val++;
		if(is_KVspecial(c)) continue;
		toput_char(blog,c);
		}
		toput_char(blog,'\n');
	}
	val=val2;
	if(on){ while(*val!='\0'){
		char8_t c=*val++;
		if(is_KVspecial(c)) continue;
		putc(c,stderr);
		}
		putc('\n',stderr);
	}
}

//Añade un '\n al final
static void log_valstr_s0(Bufferto8 *blog, bint on, const char8_t *val){
	ifunlike(*val=='\0') return;
	const char8_t *val2=val;
	if(blog!=NULL){ while(val[1]!='\0'){
		char8_t c=*val++;
		if(is_KVspecial(c)) continue;
		toput_char(blog,c);
		}
		toput_char(blog,'\n');
	}
	val=val2;
	if(on){ while(val[1]!='\0'){
		char8_t c=*val++;
		if(is_KVspecial(c)) continue;
		putc(c,stderr);
		}
		putc('\n',stderr);
	}
}

#define indent_log(blog,indent) dontimes(indent,) toput_char(blog,'\t')
#define indent_err(indent) dontimes(indent,) putc('\t',stderr)

sinline void log_line(Bufferto8 *blog, bint on, uint line){
	if(line==NO_LINE) return;
	if(blog!=NULL) towritef(blog,u8"(%u) ",line);
	if(on) fprintf(stderr,"(%u) ",line);
}
sinline void log_IL(Bufferto8 *blog, bint on, uint8m indent, uint line){
	log_indent(blog,on,indent);
	log_line(blog,on,line);
}
sinline void log_lineError(Bufferto8 *blog, bint on, uint line){
	if(blog!=NULL){
		 if(line!=NO_LINE) towritef(blog,u8"(%u) Error: ",line);
		 else towrite_string(blog,u8"Error: ");
	}
	if(on){
		if(line!=NO_LINE) fprintf(stderr,"(%u) Error: ",line);
		 else fputs("Error: ",stderr);
	}
}
sinline void log_lineWarning(Bufferto8 *blog, bint on, uint line){
	if(blog!=NULL){
		 if(line!=NO_LINE) towritef(blog,u8"(%u) Warning: ",line);
		 else towrite_string(blog,u8"Warning: ");
	}
	if(on){
		if(line!=NO_LINE) fprintf(stderr,"(%u) Warning: ",line);
		 else fputs("Warning: ",stderr);
	}
}

void log_char(Bufferto8 *blog, bint on, uint8m indent, uint line, char8_t c){
	log_IL(blog,on,indent,line);
	if(blog!=NULL) toput_char(blog,c);
	if(on) putc(c,stderr);
}
void log_s(Bufferto8 *blog, bint on, uint8m indent, uint line, const char8_t *s){
	log_IL(blog,on,indent,line);
	if(blog!=NULL) towrite_string(blog,s);
	if(on) fputs((CRP)s,stderr);
}
void log_snl(Bufferto8 *blog, bint on, uint8m indent, uint line, const char8_t *s){
	log_IL(blog,on,indent,line);
	if(blog!=NULL) towrite_stringl(blog,s,'\n');
	if(on){fputs((CRP)s,stderr); putc('\n',stderr);}
}
void log_ss(Bufferto8 *blog, bint on, uint8m indent, uint line, const char8_t *s1, const char8_t *s2){
	log_IL(blog,on,indent,line);
	if(blog!=NULL) towrite_many_strings(blog,s1,s2,NULL);
	if(on){fputs((CRP)s1,stderr); fputs((CRP)s2,stderr);}
}
void log_ssnl(Bufferto8 *blog, bint on, uint8m indent, uint line, const char8_t *s1, const char8_t *s2){
	log_IL(blog,on,indent,line);
	if(blog!=NULL) towrite_many_strings(blog,s1,s2,u8"\n",NULL);
	if(on){fputs((CRP)s1,stderr); fputs((CRP)s2,stderr); putc('\n',stderr);}
}
#define log_sss(blog,on,indent,line,...) do{log_IL(blog,on,indent,line);\
	if(blog!=NULL) towrite_many_strings(blog,__VA_ARGS__,NULL);\
	if(on) printerr_many_strings(__VA_ARGS__,NULL);\
}while(0)
//Writes a string which may be terminated by any of stn0
void log_word(Bufferto8 *blog, bint on, uint8m indent, uint line, const char8_t *s){
	log_IL(blog,on,indent,line);
	if(blog!=NULL){towrite_string_b0(blog,s); toput_char(blog,'\n');}
	if(on){while(isnot_stn0(*s)){putc(*s,stderr); s++;} putc('\n',stderr);}
}
#define log_f(blog,on,indent,line,s,...) do{log_IL(blog,on,indent,line);\
	if(blog!=NULL) towritef(blog,s,__VA_ARGS__);\
	if(on) writef_Cfile(stderr,s,__VA_ARGS__);\
}while(0)

#define LOGindent(logopts)		log_indent(	(logopts)->blog,(logopts)->on,(logopts)->indent)
#define LOG_char(logopts,ln,c)	log_char((logopts)->blog,(logopts)->on,(logopts)->indent,ln, c)
#define LOG_s(logopts,ln,s)			log_s(		(logopts)->blog,(logopts)->on,(logopts)->indent,ln, s)
#define LOG_snl(logopts,ln,s)		log_snl(	(logopts)->blog,(logopts)->on,(logopts)->indent,ln, s)
#define LOG_ss(logopts,ln,s1,s2)	log_ss(	(logopts)->blog,(logopts)->on,(logopts)->indent,ln, s1,s2)
#define LOG_ssnl(logopts,ln,s1,s2)	log_ssnl(	(logopts)->blog,(logopts)->on,(logopts)->indent,ln, s1,s2)
#define LOG_sss(logopts,ln,...)		log_sss(	(logopts)->blog,(logopts)->on,(logopts)->indent,ln, __VA_ARGS__)
#define LOG_f(logopts,ln,s,...)		log_f(		(logopts)->blog,(logopts)->on,(logopts)->indent,ln, s,__VA_ARGS__)
//
#define ChkErr_char(errors,ln,c)	iferrwar(errors,LERR) log_char((errors)->blog,(errors)->on,(errors)->indent,ln, c)
#define ChkErr_s(errors,ln,s)		iferrwar(errors,LERR) log_s(		(errors)->blog,(errors)->on,(errors)->indent,ln, s)
#define ChkErr_snl(errors,ln,s)		iferrwar(errors,LERR) log_snl(	(errors)->blog,(errors)->on,(errors)->indent,ln, s)
#define ChkErr_ss(errors,ln,s1,s2)	iferrwar(errors,LERR) log_ss(	(errors)->blog,(errors)->on,(errors)->indent,ln, s1,s2)
#define ChkErr_ssnl(errors,ln,s1,s2) iferrwar(errors,LERR)log_ssnl((errors)->blog,(errors)->on,(errors)->indent,ln, s1,s2)
#define ChkErr_sss(errors,ln,...)	iferrwar(errors,LERR) log_sss(	(errors)->blog,(errors)->on,(errors)->indent,ln, __VA_ARGS__)
#define ChkErr_f(errors,ln,s,...)		iferrwar(errors,LERR) log_f(		(errors)->blog,(errors)->on,(errors)->indent,ln, s,__VA_ARGS__)

#define Write_ERROR(errors,s,...)   Write_Error(	  (errors)->blog,(errors)->on,(errors)->indent,GET_LINE,s, __VA_ARGS__)
#define Write_WARNING(errors,s,...) Write_Warning((errors)->blog,(errors)->on,(errors)->indent,GET_LINE,s, __VA_ARGS__)
#define ERROR_F(errors,s,...)		Error_F(		errors,GET_LINE,s,__VA_ARGS__)
#define WARNING_F(errors,s,...)	Warning_F(	errors,GET_LINE,s,__VA_ARGS__)
#define ERROR_s(errors,s)			Error_s(		errors,GET_LINE,s)

//Para cuando la cadena s no es literal. En caso de que lo sea, empléese ERROR_F
#define ERROR_f(errors,s,...) do{iferrwar(errors,LWAR){\
	LOGindent(errors); log_lineError((errors)->blog,(errors)->on,GET_LINE);\
	if((errors)->blog!=NULL){towritef((errors)->blog,s,__VA_ARGS__); toput_char((errors)->blog,'\n');}\
	if((errors)->on){writef_Cfile(stderr,s,__VA_ARGS__); putc('\n',stderr);}\
}}while(0)

#define ERROR(errors,s,...) ERROR_F(errors,s,__VA_ARGS__)
#define WARNING(errors,s,...) WARNING_F(errors,s,__VA_ARGS__)

//Comprueban y aplican el index, pero no escriben el número de línea al comienzo
//ni "Error" o "Warning", ni añaden un '\n' al final
#define ChkErrLiteral(errors,s,...) do{iferrwar(errors,LERR){\
	LOGindent(errors);\
	if((errors)->blog!=NULL) towritef((errors)->blog,s,__VA_ARGS__);\
	if((errors)->on) writef_Cfile(stderr,s,__VA_ARGS__);\
}}while(0)

/* Las siguientes macros comprueban si un cierto tipo de log está activo */

#define isLevel(type,level) (globals->log.type>=level)
#define LOGgeneral(type,level,action,...) do{if(isLevel(type,level))\
	action(globals->errors.blog,globals->errors.on,globals->errors.indent,GET_LINE, __VA_ARGS__);\
}while(0)

/* actions:
c:		char
s:		string
ss:		string, string
snl;	string followed by \n
ssnl;	string, string followed by \n
word:	string ended by any of '\0', ' ', '\t, '\n'
f:		format string, ...
*/
#define LOG(type,level,action,...) LOGgeneral(type,level,log_##action,__VA_ARGS__)
#define LOGcomname(type,level) LOG(type,level,snl,KeyWords[kwass.kw].name)

static void log_cmdline(const ErrorOpts *errors, uint line, const char8_t *PTR, Bufferti8_lc *buf){
	log_IL(errors->blog,errors->on,errors->indent,line);
	Prepareline(*buf);
	if(errors->blog!=NULL) towritef(errors->blog,u8"%s %s\n",PTR,buf->pc);
	if(errors->on) fprintf(stderr,"%s %s\n",(CRP)PTR,(CRP)buf->pc);
	*buf->next=buf->savedchar;
}

#define LOG_cmd_line(type,level) if(isLevel(type,level)) log_cmdline(&globals->errors,GET_LINE,PTR,&buffer)

static void log_openfile(Globals *globals, const char8_t *fconfig){
	if(isLevel(fich,1)){
		log_indent(globals->errors.blog,globals->errors.on,globals->errors.indent);
		if(fconfig!=NULL){
			if(globals->errors.blog!=NULL) towrite_many_strings(globals->errors.blog,u8"(Archivo ",fconfig,u8"\n",NULL);
			if(globals->errors.on) printerr_many_strings(u8"(Archivo ",fconfig,u8"\n",NULL);
		}else{
			if(globals->errors.blog!=NULL) towrite_string(globals->errors.blog,u8"(Archivo anónimo\n");
			if(globals->errors.on) fprintf(stderr,(CRP)u8"(Archivo anónimo\n");
		}
	}
}

static void log_closefile(Globals *globals, const char8_t *fconfig, bool bstopped){
	if(!isLevel(fich,1)) return;
	const char8_t * const msgfin[2]={u8"fin del archivo ", u8"fin de la lectura del archivo "};
	const char8_t *pfin=msgfin[bstopped];

	log_indent(globals->errors.blog,globals->errors.on,globals->errors.indent);
	if(fconfig!=NULL){
		if(globals->errors.blog!=NULL) towrite_many_strings(globals->errors.blog,pfin,fconfig,u8")\n\n",NULL);
		if(globals->errors.on) printerr_many_strings(pfin,fconfig,u8")\n\n",NULL);
	}else{
		if(globals->errors.blog!=NULL) towrite_many_strings(globals->errors.blog,pfin,u8"anónimo)\n\n",NULL);
		if(globals->errors.on){
			if(!bstopped) fprintf(stderr,(CRP)u8"fin del archivo anónimo)\n\n");
			else fprintf(stderr,(CRP)u8"fin de la lectura del archivo anónimo)\n\n");
		}
	}
}

#define INDENT_ERROR "            "
#define INDENT_SHRT "        "

//line '\n'-ended
#define wrong_line_n(errors, line) ChkErrLiteral(errors,INDENT_ERROR u8"Al expandir la línea: %Sn\n",line)

//PTR '\0'-ended, line '\n'-ended
#define wrong_line_0n(errors, PTR, line) ChkErrLiteral(errors,INDENT_ERROR u8"Al expandir la línea: %s %Sn\n",PTR,line)

//var '\0'-ended, line '\n'-ended
#define wrong_line_eq_0n(errors, var, line) ChkErrLiteral(errors,INDENT_ERROR u8"Al expandir la asignación: %s = %Sn\n",var,line)

//text '\0'-ended
#define  wrong_text_0(errors, text) do{iferrwar(errors,LERR){\
	LOG_s(errors,NO_LINE,INDENT_ERROR u8"Al expandir el texto: ");\
	log_valstr_0((errors)->blog,(errors)->on,text);\
}}while(0)

//varname '\0'-ended, varstr '\0'-ended
#define  wrong_var_00(errors, varname, varstr) do{iferrwar(errors,LERR){\
	LOG_sss(errors,NO_LINE,INDENT_SHRT u8"Al expandir la variable: ",varname,u8" = ");\
	log_valstr_s0((errors)->blog,(errors)->on,varstr);\
}}while(0)
