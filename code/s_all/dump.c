//Establece bblank y bnl.
sinline void set_bblnl(bint *bblank, bint *bnl, const char8_t *p, uint strlen){
	*bnl=false;
	for(const char8_t *q=p; *q!='\0'; q++){
		if(*q=='\n'){*bnl=true; break;}
	}
	*bblank=(*p=='\0' || *p=='{' || is_st(*p) || is_st(p[strlen-1]));
}

//Abre adecuadamente la escritura de la cadena de reemplazo de la clave.
//Además, establece bblank y bnl.
//p apunta al caracter inicial de la cadena.
//strlen: De la cadena apuntada por p.
static void open_keystr(Bufferto8 *buf, bint *bblank, bint *bnl, const char8_t *p, uint strlen, u8int indent){
	set_bblnl(bblank,bnl,p,strlen);
	if(*bnl | *bblank) toput_char(buf,'{');
	if(*bnl){toput_char(buf,'\n'); indent_log(buf,indent);}
}

//Cierra adecuadamente la escritura de la cadena de reemplazo de la clave.
//p apunta al caracter de cierre de la cadena. Sólo se usa para mirar p[-1]=='\n' si bnl es true.
static void close_keystr(Bufferto8 *buf, bint bblank, bint bnl, const char8_t *p, u8int indent){
	if(bblank) toput_char(buf,'}');
	elif(bnl){
		if(p[-1]=='\n') toput_char(buf,'}');
		elif(indent==0) towrite_string(buf,"\n}n");
		else{toput_char(buf,'\n');
			indent_log(buf,indent);
			toput_char(buf,'}'); toput_char(buf,'n');
		}
	}
	toput_char(buf,'\n');
}

static void open_keystr_stderr(bint *bblank, bint *bnl, const char8_t *p, uint strlen, u8int indent){
	set_bblnl(bblank,bnl,p,strlen);
	if(*bnl | *bblank) putc('{',stderr);
	if(*bnl){putc('\n',stderr); indent_err(indent);}
}

//p apunta al caracter de cierre de la cadena. Sólo se usa para mirar p[-1]=='\n' si bnl es true.
static void close_keystr_stderr(bint bblank, bint bnl, const char8_t *p, u8int indent){
	if(bblank) putc('}',stderr);
	elif(bnl){
		if(p[-1]=='\n') putc('}',stderr);
		elif(indent==0) fputs("\n}n",stderr);
		else{putc('\n',stderr);
			indent_err(indent);
			putc('}',stderr); putc('n',stderr);
		}
	}
	putc('\n',stderr);
}

sinline void open_keystr_stdout(bint *bblank, bint *bnl, const char8_t *p, uint strlen){
	set_bblnl(bblank,bnl,p,strlen);
	if(*bnl | *bblank) putchar('{');
	if(*bnl) putchar('\n');
}

//p apunta al caracter de cierre de la cadena. Sólo se usa para mirar p[-1]=='\n' si bnl es true.
sinline void close_keystr_stdout(bint bblank, bint bnl, const char8_t *p){
	if(bblank) putchar('}');
	elif(bnl){
		if(p[-1]=='\n') putchar('}');
		else puts("\n}n");
	}
	putchar('\n');
}


/***-- Object Macros --***/

//str ha estar finalizada por '\0', no por ' ''\0'
//Devuelve un puntero al '\0' del final de str
sinline char8_t* do_write_objstr0(Bufferto8 *blog, iconst char8_t *str){
	while(*str!='\0'){
		char8_t c=*str++;
		if(!is_expand(c)) toput_char(blog,c);
	}
	return (ICONST char8_t *)str;
}
//str ha estar finalizada por '\0', no por ' ''\0'
//Devuelve un puntero al '\0' del final de str
//Escribe nindent '\t''s tras cada '\n'. Pero no al comienzo
static char8_t* do_write_objstr0_indent(Bufferto8 *blog, iconst char8_t *str, u8int nindent){
	while(*str!='\0'){
		char8_t c=*str++;
		if(is_expand(c)) continue;
		toput_char(blog,c);
		if(c=='\n'){dontimes(nindent,) toput_char(blog,'\t');}
	}
	return (ICONST char8_t *)str;
}

//str ha estar finalizada por '\0', no por ' ''\0'
//Devuelve un puntero al '\0' del final de str
sinline char8_t* do_write_objstr0_stdout(iconst char8_t *str){
	while(*str!='\0'){
		char8_t c=*str++;
		if(!is_expand(c)) putchar(c);
	}
	return (ICONST char8_t *)str;
}

//str ha estar finalizada por '\0', no por ' ''\0'
//Devuelve un puntero al '\0' del final de str
sinline char8_t* do_write_objstr0_stderr(iconst char8_t *str){
	while(*str!='\0'){
		char8_t c=*str++;
		if(!is_expand(c)) putc(c,stderr);
	}
	return (ICONST char8_t *)str;
}
//str ha estar finalizada por '\0', no por ' ''\0'
//Devuelve un puntero al '\0' del final de str
//Escribe nindent '\t''s tras cada '\n'. Pero no al comienzo
static char8_t* do_write_objstr0_stderr_indent(iconst char8_t *str, u8int nindent){
	while(*str!='\0'){
		char8_t c=*str++;
		if(is_expand(c)) continue;
		putc(c,stderr);
		if(c=='\n'){dontimes(nindent,) putc('\t',stderr);}
	}
	return (ICONST char8_t *)str;
}

static void dump_macobj_val(Bufferto8 *buf, const MacObj *kv, const KeyVals *kvs, u8int indent){
	bint bnl, bblank;
	char8_t *p=KeyStrp(kv);

	p[kv->c.strsize]='\0';
	open_keystr(buf,&bblank,&bnl,p,kv->c.strsize,indent);
	ifz(indent) p=do_write_objstr0(buf,p);
	else  p=do_write_objstr0_indent(buf,p,indent);
	close_keystr(buf,bblank,bnl,p,indent);
	*p=' ';
}
static void dump_macobj(Bufferto8 *buf, const MacObj *kv, const KeyVals *kvs){
	char8_t *p=KeyKeyp(kv);
	if(*p=='\0') return; //Clave eliminada
	towritef(buf,"%s = ",p);
	dump_macobj_val(buf,kv,kvs,0);
}

static void dump_macobj_val_stderr(const MacObj *kv, const KeyVals *kvs, u8int indent){
	bint bnl, bblank;
	char8_t *p=KeyStrp(kv);

	p[kv->c.strsize]='\0';
	open_keystr_stderr(&bblank,&bnl,p,kv->c.strsize,indent);
	ifz(indent) p=do_write_objstr0_stderr(p);
	else p=do_write_objstr0_stderr_indent(p,indent);
	close_keystr_stderr(bblank,bnl,p,indent);
	*p=' ';
}
static void dump_macobj_stderr(const MacObj *kv, const KeyVals *kvs){
	char8_t *p=KeyKeyp(kv);
	if(*p=='\0') return; //Clave eliminada
	fprintf(stderr,"%s = ",p);
	dump_macobj_val_stderr(kv,kvs,0);
}

static void dump_macobj_val_stdout(const MacObj *kv, const KeyVals *kvs){
	bint bnl, bblank;
	char8_t *p=KeyStrp(kv);

	p[kv->c.strsize]='\0';
	open_keystr_stdout(&bblank,&bnl,p,kv->c.strsize);
	p=do_write_objstr0_stdout(p);
	close_keystr_stdout(bblank,bnl,p);
	*p=' ';
}
static void dump_macobj_stdout(const MacObj *kv, const KeyVals *kvs){
	char8_t *p=KeyKeyp(kv);
	if(*p=='\0') return; //Clave eliminada
	printf("%s = ",p);
	dump_macobj_val_stdout(kv,kvs);
}

static void LOGmacobj(Globals *globals,const MacObj *kv, bint bindent){
	const char8_t *p=KeyKeyg(kv);
	if(*p=='\0') return; //Clave eliminada
	u8int indent= bindent ? globals->errors.indent : 0;

	indent_log(globals->errors.blog,indent);
	towritef(globals->errors.blog,"%s [%s] = ",p,TypesStrings[kv->value.type]);
	dump_macobj_val(globals->errors.blog,kv,&globals->kvs,indent);
}
static void LOGmacobj_err(Globals *globals,const MacObj *kv, bint bindent){
	const char8_t *p=KeyKeyg(kv);
	if(*p=='\0') return; //Clave eliminada
	u8int indent= bindent ? globals->errors.indent : 0;

	indent_err(indent);
	fprintf(stderr,"%s [%s] = ",p,TypesStrings[kv->value.type]);
	dump_macobj_val_stderr(kv,&globals->kvs,indent);
}


/***-- Function Macros --***/

//narg empieza a contar en 0.
sinline const char8_t* get_arg(const char8_t *args, uint8m narg){
	while(narg!=0){while(*args!='\0') args++; args++; narg--;}
	return args;
}

//str ha estar finalizada por '\0', no por ' ''\0'
//Devuelve un puntero al '\0' del final de str
//Escribe nindent '\t''s tras cada '\n'. Pero no al comienzo
static char8_t* do_write_funstr0_indent(Bufferto8 *blog, iconst char8_t *str, const char8_t *arg_names, u8int nindent){
	while(*str!='\0'){
		char8_t c=*str++;
		if(is_expand(c)) continue;
		if(c!=KV_Substitute){
			toput_char(blog,c);
			if(c=='\n'){dontimes(nindent,) toput_char(blog,'\t');}
		}else{
			toput_char(blog,'#');
			const char8_t *arg=get_arg(arg_names,*str++-'1');
			toput_char(blog,'('); towrite8_string(blog,arg); toput_char(blog,')');
		}
	}
	return (ICONST char8_t *)str;
}

//str ha estar finalizada por '\0', no por ' ''\0'
//Devuelve un puntero al '\0' del final de str
sinline char8_t* do_write_funstr0_stdout(iconst char8_t *str, const char8_t *arg_names){
	while(*str!='\0'){
		char8_t c=*str++;
		if(!is_cmdchar(c)) putchar(c);
		elif(c==KV_Substitute){
			putchar('#');
			const char8_t *arg=get_arg(arg_names,*str++-'1');
			putchar('('); fputs(arg,stdout); putchar(')');
		}
	}
	return (ICONST char8_t *)str;
}

//str ha estar finalizada por '\0', no por ' ''\0'
//Devuelve un puntero al '\0' del final de str
//Escribe nindent '\t''s tras cada '\n'. Pero no al comienzo
static char8_t* do_write_funstr0_stderr_indent(iconst char8_t *str, const char8_t *arg_names, u8int nindent){
	while(*str!='\0'){
		char8_t c=*str++;
		if(is_expand(c)) continue;
		if(c!=KV_Substitute){
			putc(c,stderr);
			if(c=='\n'){dontimes(nindent,) putc('\t',stderr);}
		}else{
			putc('#',stderr);
			const char8_t *arg=get_arg(arg_names,*str++-'1');
			putc('(',stderr); fputs(arg,stderr); putc(')',stderr);
		}
	}
	return (ICONST char8_t *)str;
}

static void dump_macfun_args(Bufferto8 *buf, const MacFun *kv, const KeyVals *kvs){
	uint8m n=kv->nargs;
	if(n==0){towrite_string(buf,"() = "); return;}

	 n--;
	const char8_t *arg=kvs->strs.ppio+kv->args;
	toput_char(buf,'('); towrite_string(buf,arg);
	while(n>=1){n--;
		while(*arg!='\0') arg++; arg++;
		toput_char(buf,','); towrite_string(buf,arg);
	}
	towrite_string(buf,") = ");
}

static void dump_macfun_val(Bufferto8 *buf, const MacFun *kv, const KeyVals *kvs, u8int indent){
	bint bnl, bblank;
	char8_t *p=KeyStrp(kv);

	p[kv->c.strsize]='\0';
	open_keystr(buf,&bblank,&bnl,p,kv->c.strsize,indent);
	p=do_write_funstr0_indent(buf,p,kvs->strs.ppio+kv->args,indent);
	close_keystr(buf,bblank,bnl,p,indent);
	*p=' ';
}
static void dump_macfun(Bufferto8 *buf, const MacFun *kv, const KeyVals *kvs){
	char8_t *p=KeyKeyp(kv);
	if(*p=='\0') return; //Clave eliminada

	towrite_string(buf,p);
	dump_macfun_args(buf,kv,kvs);
	dump_macfun_val(buf,kv,kvs,0);
}

static void dump_macfun_args_FILE(const MacFun *kv, const KeyVals *kvs, FILE *strm){
	uint8m n=kv->nargs;
	if(n==0){fputs("() = ",strm); return;}

	n--;
	const char8_t *arg=kvs->strs.ppio+kv->args;
	putc('(',strm); fputs(arg,strm);
	while(n>=1){n--;
		while(*arg!='\0') arg++; arg++;
			putc(',',strm); fputs(arg,strm);
	}
	fputs(") = ",strm);
}

static void dump_macfun_val_stderr(const MacFun *kv, const KeyVals *kvs, u8int indent){
	bint bnl, bblank;
	char8_t *p=KeyStrp(kv);

	p[kv->c.strsize]='\0';
	open_keystr_stderr(&bblank,&bnl,p,kv->c.strsize,indent);
	p=do_write_funstr0_stderr_indent(p,kvs->strs.ppio+kv->args,indent);
	close_keystr_stderr(bblank,bnl,p,indent);
	*p=' ';
}
static void dump_macfun_stderr(const MacFun *kv, const KeyVals *kvs){
	char8_t *p=KeyKeyp(kv);
	if(*p=='\0') return; //Clave eliminada
	fputs(p,stderr);
	dump_macfun_args_FILE(kv,kvs,stderr);
	dump_macfun_val_stderr(kv,kvs,0);
}

static void dump_macfun_val_stdout(const MacFun *kv, const KeyVals *kvs){
	bint bnl, bblank;
	char8_t *p=KeyStrp(kv);

	p[kv->c.strsize]='\0';
	open_keystr_stdout(&bblank,&bnl,p,kv->c.strsize);
	p=do_write_funstr0_stdout(p,kvs->strs.ppio+kv->args);
	close_keystr_stdout(bblank,bnl,p);
	*p=' ';
}
static void dump_macfun_stdout(const MacFun *kv, const KeyVals *kvs){
	char8_t *p=KeyKeyp(kv);
	if(*p=='\0') return; //Clave eliminada
	fputs(p,stdout);
	dump_macfun_args_FILE(kv,kvs,stdout);
	dump_macfun_val_stdout(kv,kvs);
}

static void LOGmacfun(Globals *globals,const MacFun *kv, bint bindent){
	const char8_t *p=KeyKeyg(kv);
	if(*p=='\0') return; //Clave eliminada
	u8int indent= bindent ? globals->errors.indent : 0;

	indent_log(globals->errors.blog,indent);
	towrite_string(globals->errors.blog,p);
	dump_macfun_args(globals->errors.blog,kv,&globals->kvs);
	dump_macfun_val(globals->errors.blog,kv,&globals->kvs,indent);
}
static void LOGmacfun_err(Globals *globals,const MacFun *kv, bint bindent){
	const char8_t *p=KeyKeyg(kv);
	if(*p=='\0') return; //Clave eliminada
	u8int indent= bindent ? globals->errors.indent : 0;

	indent_err(indent);
	fputs(p,stderr);
	dump_macfun_args_FILE(kv,&globals->kvs,stderr);
	dump_macfun_val_stderr(kv,&globals->kvs,indent);
}


/***-- Dump todas las kvs --***/

int dump_kvs(const char8_t *file, KeyVals *kvs){
	int nret;
	Bufferto8 buf;

	ifnzunlike(nret=toopen_utf8(&buf,file)) return nret;
	{durchVectorp(MacObj,kvs->objs){
		dump_macobj(&buf,p,kvs);
	}}
	{durchVectorp(MacFun,kvs->funcs){
		dump_macfun(&buf,p,kvs);
	}}
	toclose(&buf);
	return 0;
}
int dump_kvs_online(KeyVals *kvs){
	{durchVectorp(MacObj,kvs->objs){
		dump_macobj_stdout(p,kvs);
	}}
	{durchVectorp(MacFun,kvs->funcs){
		dump_macfun_stdout(p,kvs);
	}}
	return 0;
}

