sinline void messa_error_asignar(FConfigParse *fparse, const MacObj* kv, umint type){
	Error_F(&fparse->errors,NO_LINE,u8"No se puede asignar el campo %s. No es un %s",KeyKey(fparse,kv),TypesStrings[type]);
}

char8_t* String___keyval(MacObj *kv, FConfigParse *fparse, char8_t *s, uint n){
	char8_t *pe=KeyStr(fparse,kv);
	char8_t c;
	uint nn;

	if(kv->c.strsize>=n){
		nn=--n;
		char8_t *pfin=pe+nn;
		while(pfin>pe && (*pfin&0xC0)==0x80) pfin--, nn--; //utf-8
		c=pe[nn], pe[nn]='\0';

		uint m=min(6,nn);
		pfin-=m;
		while(pfin>pe && (*pfin&0xC0)==0x80) pfin--;

		Error_F(&fparse->errors,NO_LINE,u8"El texto para %s es demasiado largo. Se permiten como máximo %u caracteres. El texto indicado tiene %u."
					u8" Se ha copiado hasta ... %s",KeyStr(fparse,kv),n,kv->c.strsize,pfin);
		fparse->errors.err=ERR_VAL_StringTooLong;
	}else{
		nn=kv->c.strsize;
		c=pe[kv->c.strsize], pe[kv->c.strsize]='\0';
		fparse->errors.err=0;
	}

	s=strpcpy8(s,pe);
	pe[nn]=c;
	return s;
}

void Bool___keyval(MacObj *kv, FConfigParse *fparse){
	ifunlike(kv->value.type==VTYPE_erroneous_bool){
		messa_error_asignar(fparse,kv,VAL_TYPE_bool);
		return;
	}
	if(kv->value.type==VAL_TYPE_bool) return; //El valor ya está. No hay nada que hacer

	const char8_t *pc=KeyStr(fparse,kv);
	BoolValidado d=lee_valida_Bool(&pc);
	iflike(d.err>=0){kv->value.type=VAL_TYPE_bool;  kv->value.b=d.val;}
	else kv->value.type=VTYPE_erroneous_bool;
	fparse->errors.err=d.err;
	error_kvBool(fparse,kv);
}

void Entero___keyval(MacObj *kv, FConfigParse *fparse, uint max_val, uint max_recom){
	EnteroValidado d;

	ifunlike(kv->value.type==VTYPE_erroneous_uint){
		messa_error_asignar(fparse,kv,VAL_TYPE_uint);
		return;
	}
	if(kv->value.type==VAL_TYPE_uint){ //just check it
		d.val=kv->value.val, d.err=0;
		check_Entero_range(&d.err,d.val,max_val,max_recom);
	}else{
		const char8_t *pc=KeyStr(fparse,kv);
		d=lee_valida_Entero(&pc,max_val,max_recom);
		iflike(d.err>=0){kv->value.type=VAL_TYPE_uint;  kv->value.val=d.val;}
		else kv->value.type=VTYPE_erroneous_uint;
	}
	fparse->errors.err=d.err;
	error_kvEntero(fparse,kv,max_val,max_recom);
}

void Medida___keyval(MacObj *kv, FConfigParse *fparse, uint max_val, uint max_recom){
	MagnitudValidada d;

	ifunlike(kv->value.type==VTYPE_erroneous_sMedida){
		messa_error_asignar(fparse,kv,VAL_TYPE_sMedida);
		return;
	}
	if(kv->value.type==VAL_TYPE_sMedida){ //just check it
		d.val=kv->value.sval;
		ifunlike(kv->value.sval<0) d.err=ERR_VAL_NEG;
		else{d.err=0; check_uMagnitud_range(&d.err,d.val,max_val,max_recom);}
	}else{
		const char8_t *pc=KeyStr(fparse,kv);
		d=lee_valida_uLongitudTabla(&pc,max_val,max_recom,UNI_DMM);
		if(d.err>=0){kv->value.type=VAL_TYPE_sMedida; kv->value.sval=(ssint)d.val;}
		else kv->value.type=VTYPE_erroneous_sMedida;
	}
	fparse->errors.err=d.err;
	error_kvsMedida(fparse,kv,max_val,max_recom);
}

void sMedida___keyval(MacObj *kv, FConfigParse *fparse, uint max_val, uint max_recom){
	sMagnitudValidada d;

	ifunlike(kv->value.type==VTYPE_erroneous_sMedida){
		messa_error_asignar(fparse,kv,VAL_TYPE_sMedida);
		return;
	}
	if(kv->value.type==VAL_TYPE_sMedida){ //just check it
		d.val=kv->value.sval, d.err=0;
		check_sMagnitud_range(&d.err,d.val,max_val,max_recom);
	}else{
		const char8_t *pc=KeyStr(fparse,kv);
		d=lee_valida_sLongitudTabla(&pc,max_val,max_recom,UNI_DMM);
		if(d.err>=0){kv->value.type=VAL_TYPE_sMedida; kv->value.sval=d.val;}
		else kv->value.type=VTYPE_erroneous_sMedida;
	}
	fparse->errors.err=d.err;
	error_kvsMedida(fparse,kv,max_val,max_recom);
}

void uFloat___keyval(MacObj *kv, FConfigParse *fparse, float max_val, float max_recom){
	FloatValidado d;

	ifunlike(kv->value.type==VTYPE_erroneous_float){
		messa_error_asignar(fparse,kv,VAL_TYPE_float);
		return;
	}
	if(kv->value.type==VAL_TYPE_float){ //just check it
		d.val=kv->value.fl, d.err=0;
		check_uFloat_range(&d.err,d.val,max_val,max_recom);
	}else{
		const char8_t *pc=KeyStr(fparse,kv);
		d=lee_valida_uFloat(&pc,max_val,max_recom);
		iflike(d.err>=0){kv->value.type=VAL_TYPE_float;  kv->value.fl=d.val;}
		else kv->value.type=VTYPE_erroneous_float;
	}
	fparse->errors.err=d.err;
	error_kvFloat(fparse,kv,max_val,max_recom);
}

void sFloat___keyval(MacObj *kv, FConfigParse *fparse, float max_val, float max_recom){
	FloatValidado d;

	ifunlike(kv->value.type==VTYPE_erroneous_float){
		messa_error_asignar(fparse,kv,VAL_TYPE_float);
		return;
	}
	if(kv->value.type==VAL_TYPE_float){ //just check it
		d.val=kv->value.fl, d.err=0;
		check_sFloat_range(&d.err,d.val,max_val,max_recom);
	}else{
		const char8_t *pc=KeyStr(fparse,kv);
		d=lee_valida_uFloat(&pc,max_val,max_recom);
		iflike(d.err>=0){kv->value.type=VAL_TYPE_float;  kv->value.fl=d.val;}
		else kv->value.type=VTYPE_erroneous_float;
	}
	fparse->errors.err=d.err;
	error_kvFloat(fparse,kv,max_val,max_recom);
}
