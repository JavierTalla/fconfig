#pragma once
/* Escribir errores y warnings al fichero de log */

//LERR y LWAR han de coincidir con los valores de FConfigParse_ErrOutput
#define LERR 1
#define LWAR 2
#define NO_LINE Я32

#if FILE8BITS_ARE_UTF8
	#define towrite_filename towrite_string
#else
	#define towrite_filename towrite_string8_utf8
#endif

//Whether the for-Buffert0 string s can be sent to printf
sinline bint is_s_printf(const char8_t *s){
	while(*s){if(*s=='%' && (s[1]=='S' || s[1]=='w')) return false;}
	return true;
}

//Writes to file a string formatted for towritef
extern void writef_Cfile(FILE *const file, const char8_t *str, ...);

//indent: Si =0, no escribe nada.
//Si !=0, escribe "\t...\t", con indent '\t'.
sinline void log_indent(Bufferto8 *blog, bint on, uint8m indent){
	if(blog!=NULL){dontimes(indent,) toput_char(blog,'\t');}
	if(on){dontimes(indent,) putc('\t',stderr);}
}

#define biuld_Diagnostric(Diag,s) "(%u)" Diag ": " s "\n"
#define build_Error(s) biuld_Diagnostric("Error",s)
#define build_Warning(s) biuld_Diagnostric("Warning",s)

#define Write_Diagnostic(blog,on,line,Diag,s,...) \
	if(line==NO_LINE){\
		if(blog!=NULL) towritef(blog, Diag u8": " s "\n", __VA_ARGS__); \
		if(on) writef_Cfile(stderr, Diag u8": " s "\n", __VA_ARGS__); \
	}else{\
		towritef(blog, u8"(%u) " Diag ": " s "\n", line, __VA_ARGS__); \
		if(on) writef_Cfile(stderr, u8"(%u) " Diag ": " s "\n", line, __VA_ARGS__);\
	}
#define Write_Error(blog,on,indent,line,s,...) do{log_indent(blog,on,indent); Write_Diagnostic(blog,on,line,"Error",s, __VA_ARGS__)}while(0)
#define Write_Warning(blog,on,indent,line,s,...) do{log_indent(blog,on,indent); Write_Diagnostic(blog,on,line,"Warning",s, __VA_ARGS__)}while(0)

//s ha de ser una cadena literal
#define Error_F(errors,line,s,...) if((errors)->err_war>=LERR) Write_Error((errors)->blog,(errors)->on,(errors)->indent, line,s, __VA_ARGS__)
#define Warning_F(errors,line,s,...) if((errors)->err_war>=LWAR) Write_Warning((errors)->blog,(errors)->on,(errors)->indent, line,s, __VA_ARGS__)
//s puede no ser una cadena literal
#define Error_s(errors,line,s) if((errors)->err_war>=LERR) Write_Error((errors)->blog,(errors)->on,(errors)->indent,line,"%s",s)
#define Warning_s(errors,line,s) if((errors)->err_war>=LWAR) Write_Warning((errors)->blog,(errors)->on,(errors)->indent,line,"%s",s)
