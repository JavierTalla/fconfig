BoolValidado lee_valida_Bool(iconst char8_t **pc){
	BoolValidado d;
	ifunlike(is_stn0(**pc)){d.err=ERR_VAL_EMPTY; return d;}

	d.val=lee_Bool(pc);
	ifunlike(isnot_stn0(**pc)) d.err=ERR_VAL_NOBOOLEAN;
	else d.err=0;
	return d;
}

EnteroValidado lee_valida_Entero(iconst char8_t **pc, uint max_val, uint max_recom){
	EnteroValidado d;
	ifunlike(is_stn0(**pc)){d.err=ERR_VAL_EMPTY; return d;}

	d.val=uint_check___str8(pc);
	ifunlike(isnot_stn0(**pc)){
		while((char8_t)(**pc-'0')<10) (*pc)++;
		if(is_stn0(**pc)) d.err=ERR_VAL_NUMHUGE;
		else d.err=ERR_VAL_NONUMBER;
		return d;
	}
	d.err=0;

	check_Entero_range(&d.err,d.val,max_val,max_recom);
	return d;
}

MagnitudValidada lee_valida_LongitudTabla(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni){
	MagnitudValidada d;
	ifunlike(is_stn0(**pc)){d.err=ERR_VAL_EMPTY; return d;}
	ifunlike((char8_t)(**pc-'0')>=10){d.err=ERR_VAL_NONUMBER; return d;}

	LongitudTabla tl=lee_Magnitud_decimales(pc,Magnitud_Tabla);
	d.err=0;
	d.val=tl.val;

	ifunlike(tl.val==MAX_MAGNI_VAL) d.err=ERR_VAL_NUMHUGE;
	elifunlike(tl.uni==UNI_ABSENT) d.err=ERR_UNI_ABSENT;
	elifunlike(tl.uni==UNI_WRONG || tl.uni==UNI_INVALID) d.err=ERR_UNI_WRONG;
	ifunlike(d.err) return d;

	//Mirar si cabe en un uint al pasarlo a 'uni'.
	//El valor máximo posible válido de tl.val es MAX_MAGNI_VAL = 2^32/256 - 2
	switch(uni){
	case UNI_DMM:
		if(tl.uni==UNI_M){ //Por tanto sólo hace falta comprobar en caso de UNI_M
			ifunlike(d.val>(0xFFFFffff/10000)){d.err=ERR_VAL_HUGE; goto checkval;}  // ~430.000
		}
		switch(tl.uni){
			case UNI_DMM: break;
			case UNI_MM: d.val*=10; break;
			case UNI_CM: d.val*=100; break;
			case UNI_M: d.val*=10000; break;
		}
		break;
	case UNI_MM:
		if(tl.uni==UNI_M){ //Por tanto sólo hace falta comprobar en caso de UNI_M
			ifunlike(d.val>(0xFFFFffff/1000)){d.err=ERR_VAL_HUGE; goto checkval;}  // ~4.300.000
		}
		switch(tl.uni){
			case UNI_DMM: d.val/=10; if(10*d.val!=tl.val) tl.res=MAGRES_ROUND; break;
			case UNI_CM: d.val*=10; break;
			case UNI_M: d.val*=1000; break;
		}
		break;
	case UNI_CM:
		//Siempre cabrá en un uint al pasarlo a cm.
		switch(tl.uni){
			case UNI_DMM: d.val/=100; if(100*d.val!=tl.val) tl.res=MAGRES_ROUND; break;
			case UNI_MM: d.val/=10; if(10*d.val!=tl.val) tl.res=MAGRES_ROUND; break;
			case UNI_M: d.val*=100; break;
		}
	}

	if(d.err==0 && tl.res==MAGRES_ROUND) d.err=WAR_VAL_RES;
checkval:
	check_uMagnitud_range(&d.err,d.val,max_val,max_recom);
	return d;
}

MagnitudValidada lee_valida_uLongitudTabla(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni){
	const char8_t * const pstr=*pc;
	MagnitudValidada d=lee_valida_LongitudTabla(pc,max_val,max_recom,uni);
	ifunlike(d.err==ERR_VAL_NONUMBER){if(*pstr=='-' && (char8_t)(pstr[1]-'0')<10) d.err=ERR_VAL_NEG;}
	return d;
}

sMagnitudValidada lee_valida_sLongitudTabla(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni){
	bint bneg=0; if(**pc=='-') bneg=1, (*pc)++;
	MagnitudValidada d=lee_valida_LongitudTabla(pc,max_val,max_recom,uni);
	sMagnitudValidada sd=(sMagnitudValidada){.val=(ssint)d.val,.err=d.err};
	ifunlike(d.err!=ERR_VAL_NONUMBER && bneg) sd.val=-sd.val;
	return sd;
}

/*Las unidades en las que esté exresado el dato tienen que ser m o cm.
'uni' puede ser UNI_M o UNI_CM.*/
MagnitudValidada lee_valida_LongitudTierraZ(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni){
	MagnitudValidada d;
	ifunlike(is_stn0(**pc)){d.err=ERR_VAL_EMPTY; return d;}
	ifunlike((char8_t)(**pc-'0')>=10){d.err=ERR_VAL_NONUMBER; return d;}

	LongitudTabla tl=lee_Magnitud_decimales(pc,Magnitud_TierraZ);
	d.err=0;
	d.val=tl.val;

	ifunlike(tl.val==MAX_MAGNI_VAL) d.err=ERR_VAL_NUMHUGE;
	elifunlike(tl.uni==UNI_ABSENT) d.err=ERR_UNI_ABSENT;
	elifunlike(tl.uni==UNI_WRONG || tl.uni==UNI_INVALID) d.err=ERR_UNI_WRONG;
	ifunlike(d.err) return d;

	//Mirar si cabe en un uint al pasarlo a 'uni'.
	//El valor máximo posible válido de tl.val es MAX_MAGNI_VAL = 2^32/256 - 2
	switch(uni){
	case UNI_CM:
		if(tl.uni==UNI_M) d.val*=100;
		break;
	case UNI_M:
		if(tl.uni== UNI_CM){d.val/=100; if(100*d.val!=tl.val) tl.res=MAGRES_ROUND;}
		break;
	}

	if(d.err==0 && tl.res==MAGRES_ROUND) d.err=WAR_VAL_RES;
	check_uMagnitud_range(&d.err,d.val,max_val,max_recom);
	return d;
}

MagnitudValidada lee_valida_uLongitudTierraZ(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni){
	const char8_t * const pstr=*pc;
	MagnitudValidada d=lee_valida_LongitudTierraZ(pc,max_val,max_recom,uni);
	ifunlike(d.err==ERR_VAL_NONUMBER){if(*pstr=='-' && (char8_t)(pstr[1]-'0')<10) d.err=ERR_VAL_NEG;}
	return d;
}

sMagnitudValidada lee_valida_sLongitudTierraZ(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni){
	bint bneg=0; if(**pc=='-') bneg=1, (*pc)++;
	MagnitudValidada d=lee_valida_LongitudTierraZ(pc,max_val,max_recom,uni);
	sMagnitudValidada sd=(sMagnitudValidada){.val=(ssint)d.val,.err=d.err};
	ifunlike(d.err!=ERR_VAL_NONUMBER && bneg) sd.val=-sd.val;
	return sd;
}

/* Las unidades en las que esté exresado el dato pueden ser m, cm, segundos de arco, terceros de arco o décimas de tercero.
'uni' puede ser UNI_M o UNI_CM */
MagnitudValidada lee_valida_LongitudTierraPeq(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni){
	MagnitudValidada d;
	ifunlike(is_stn0(**pc)){d.err=ERR_VAL_EMPTY; return d;}
	ifunlike((char8_t)(**pc-'0')>=10){d.err=ERR_VAL_NONUMBER; return d;}

	LongitudTabla tl=lee_Magnitud_decimales(pc,Magnitud_TierraPeq);
	d.err=0;
	d.val=tl.val;

	ifunlike(tl.val==MAX_MAGNI_VAL) d.err=ERR_VAL_NUMHUGE;
	elifunlike(tl.uni==UNI_ABSENT) d.err=ERR_UNI_ABSENT;
	elifunlike(tl.uni==UNI_WRONG || tl.uni==UNI_INVALID) d.err=ERR_UNI_WRONG;
	ifunlike(d.err) return d;

	//Mirar si cabe en un uint al pasarlo a 'uni'.
	//El valor máximo posible válido de tl.val es MAX_MAGNI_VAL = 2^32/256 - 2
	switch(uni){
	case UNI_CM:
		if(tl.uni==UNI_ARC_SEG){ //Por tanto sólo hace falta comprobar en caso de UNI_ARC_SEG
			ifunlike(d.val>(0xFFFFffff/3000)){d.err=ERR_VAL_HUGE; goto checkval;}  // ~1.430.000
		}
		switch(tl.uni){
			case UNI_M: d.val*=100; break;
			case UNI_ARC_SEG: d.val*=3000; break; //30m
			case UNI_ARC_TER: d.val*=50; break;
			case UNI_ARC_DTER: d.val*=5; break;
		}
		break;
	case UNI_M:
		switch(tl.uni){
			case UNI_CM: d.val/=100; if(100*d.val!=tl.val) tl.res=MAGRES_ROUND; break;
			case UNI_ARC_SEG: d.val*=30; break;
			case UNI_ARC_TER: if(d.val&1) tl.res=MAGRES_ROUND; d.val>>=1; break;
			case UNI_ARC_DTER: d.val/=20; if(100*d.val!=tl.val) tl.res=MAGRES_ROUND; break;
		}
		break;
	}

	if(d.err==0 && tl.res==MAGRES_ROUND) d.err=WAR_VAL_RES;
checkval:
	check_uMagnitud_range(&d.err,d.val,max_val,max_recom);
	return d;
}

MagnitudValidada lee_valida_uLongitudTierraPeq(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni){
	const char8_t * const pstr=*pc;
	MagnitudValidada d=lee_valida_LongitudTierraPeq(pc,max_val,max_recom,uni);
	ifunlike(d.err==ERR_VAL_NONUMBER){if(*pstr=='-' && (char8_t)(pstr[1]-'0')<10) d.err=ERR_VAL_NEG;}
	return d;
}

sMagnitudValidada lee_valida_sLongitudTierraPeq(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni){
	bint bneg=0; if(**pc=='-') bneg=1, (*pc)++;
	MagnitudValidada d=lee_valida_LongitudTierraPeq(pc,max_val,max_recom,uni);
	sMagnitudValidada sd=(sMagnitudValidada){.val=(ssint)d.val,.err=d.err};
	ifunlike(d.err!=ERR_VAL_NONUMBER && bneg) sd.val=-sd.val;
	return sd;
}

/* Las unidades en las que esté exresado el dato pueden ser m, km, grados o segundos de arco.
'uni' puede ser UNI_KM o UNI_M */
MagnitudValidada lee_valida_LongitudTierraGra(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni){
	MagnitudValidada d;
	ifunlike(is_stn0(**pc)){d.err=ERR_VAL_EMPTY; return d;}
	ifunlike((char8_t)(**pc-'0')>=10){d.err=ERR_VAL_NONUMBER; return d;}

	LongitudTabla tl=lee_Magnitud_decimales(pc,Magnitud_TierraGrande);
	d.err=0;
	d.val=tl.val;

	ifunlike(tl.val==MAX_MAGNI_VAL) d.err=ERR_VAL_NUMHUGE;
	elifunlike(tl.uni==UNI_ABSENT) d.err=ERR_UNI_ABSENT;
	elifunlike(tl.uni==UNI_WRONG || tl.uni==UNI_INVALID) d.err=ERR_UNI_WRONG;
	ifunlike(d.err) return d;

	//Mirar si cabe en un uint al pasarlo a 'uni'.
	//El valor máximo posible válido de tl.val es MAX_MAGNI_VAL = 2^32/256 - 2
	switch(uni){
	case UNI_M:
		if(tl.uni==UNI_ARC_GRADOS){
			ifunlike(d.val>(0xFFFFffff/(3600*30))){d.err=ERR_VAL_HUGE; goto checkval;}  // ~40.000
		}elif(tl.uni==UNI_KM){
			ifunlike(d.val>(0xFFFFffff/1000)){d.err=ERR_VAL_HUGE; goto checkval;}  // ~4.300.000
		}
		switch(tl.uni){
			case UNI_KM: d.val*=1000; break;
			case UNI_ARC_SEG: d.val*=30; break;
			case UNI_ARC_GRADOS: d.val*=3600*30; break;
		}
		break;
	case UNI_KM:
		switch(tl.uni){
			case UNI_M: d.val/=1000; if(1000*d.val!=tl.val) tl.res=MAGRES_ROUND; break;
			case UNI_ARC_SEG: d.val*=3; d.val/=100;  if(100*d.val!=3*tl.val) tl.res=MAGRES_ROUND; break;
			case UNI_ARC_GRADOS: d.val*=108; break;
		}
		break;
	}

	if(d.err==0 && tl.res==MAGRES_ROUND) d.err=WAR_VAL_RES;
checkval:
	check_uMagnitud_range(&d.err,d.val,max_val,max_recom);
	return d;
}

MagnitudValidada lee_valida_uLongitudTierraGra(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni){
	const char8_t * const pstr=*pc;
	MagnitudValidada d=lee_valida_LongitudTierraGra(pc,max_val,max_recom,uni);
	ifunlike(d.err==ERR_VAL_NONUMBER){if(*pstr=='-' && (char8_t)(pstr[1]-'0')<10) d.err=ERR_VAL_NEG;}
	return d;
}

/* Las unidades en las que esté expresado el dato pueden ser UNI_T_MIN o UNI_T_SEC.
'uni' puede ser UNI_T_MSEC o UNI_T_SEC */
MagnitudValidada lee_valida_Tiempo(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni){
	MagnitudValidada d;
	ifunlike(is_stn0(**pc)){d.err=ERR_VAL_EMPTY; return d;}
	ifunlike((char8_t)(**pc-'0')>=10){d.err=ERR_VAL_NONUMBER; return d;}

	Tiempo tl=lee_Magnitud_decimales(pc,Magnitud_Tiempo);
	d.err=0;
	d.val=tl.val;
	if(tl.val==0) return d; //Si se indicó un 0 ignoramos las unidades.

	ifunlike(tl.val==MAX_MAGNI_VAL) d.err=ERR_VAL_NUMHUGE;
	elifunlike(tl.uni==UNI_ABSENT) d.err=ERR_UNI_ABSENT;
	elifunlike(tl.uni==UNI_WRONG || tl.uni==UNI_INVALID) d.err=ERR_UNI_WRONG;
	ifunlike(d.err) return d;

	//Mirar si cabe en un uint al pasarlo a 'uni'.
	//El valor máximo posible válildo de tl.val es MAX_MAGNI_VAL = 2^32/256 - 2
	switch(uni){
	case UNI_T_MSEC:
		if(tl.uni==UNI_T_MIN){
			ifunlike(d.val>(0xFFFFffff/60000)){d.err=ERR_VAL_HUGE; goto checkval;}  // ~72.000
			d.val*=60000;
		}elif(tl.uni==UNI_T_SEC){
			ifunlike(d.val>(0xFFFFffff/1000)){d.err=ERR_VAL_HUGE; goto checkval;}  // ~4.300.000
			d.val*=1000;
		}
		break;
	case UNI_T_SEC:
		//Siempre cabrá en un uint al pasarlo a SEC
		if(tl.uni==UNI_T_MSEC){d.val/=1000; if(1000*d.val!=tl.val) tl.res=1;}
		elif(tl.uni==UNI_T_MIN) d.val*=60;
		break;
	}

checkval:
	check_uMagnitud_range(&d.err,d.val,max_val,max_recom);
	return d;
}

MagnitudValidada lee_valida_uTiempo(iconst char8_t **pc, uint max_val, uint max_recom, uint8m uni){
	const char8_t * const pstr=*pc;
	MagnitudValidada d=lee_valida_Tiempo(pc,max_val,max_recom,uni);
	ifunlike(d.err==ERR_VAL_NONUMBER){if(*pstr=='-' && (char8_t)(pstr[1]-'0')<10) d.err=ERR_VAL_NEG;}
	return d;
}

FloatValidado lee_valida_uFloat(iconst char8_t **pc, float max_val, float max_recom){
	FloatValidado d;
	ifunlike(is_stn0(**pc)){d.err=ERR_VAL_EMPTY; return d;}

	bint bneg=0; if(**pc=='-') bneg=1, (*pc)++;
	double x=double___str8(pc);
	ifunlike(isnot_stn0(**pc)) d.err=ERR_VAL_NONUMBER;
	elifunlike(bneg){
		if(x<0) d.err=ERR_VAL_NONUMBER;
		else d.err=ERR_VAL_NEG;
	}
	elifunlike(x>FLT_MAX) d.err=ERR_VAL_HUGE;
	else{d.val=(float)x; d.err=0;}

	check_uFloat_range(&d.err,d.val,max_val,max_recom);
	return d;
}

FloatValidado lee_valida_sFloat(iconst char8_t **pc, float max_val, float max_recom){
	FloatValidado d;
	ifunlike(is_stn0(**pc)){d.err=ERR_VAL_EMPTY; return d;}

	double x=double___str8(pc);
	ifunlike(isnot_stn0(**pc)) d.err=ERR_VAL_NONUMBER;
	elifunlike(x>FLT_MAX) d.err=ERR_VAL_HUGE;
	else{d.val=(float)x; d.err=0;}

	check_sFloat_range(&d.err,d.val,max_val,max_recom);
	return d;
}
