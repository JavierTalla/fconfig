//These are for buffer.pc pointing to a stn0-ended word
//La palabra no puede estar terminada por '%'; es decir, no puede aparecer un comentario
//inmediatamente pegado a una palabra, pues se tomaría como parte de ella.
#define isVALpc(s) (strcmp8_b(*pc,s)==0)
#define ifVALpc(s) if(isVALpc(s))
#define elifVALpc(s) else ifVALpc(s)

/*Parsea la cadena apuntada por *pc. Devuelve un puntero al carácter que detuvo la lectura.
El único valor que puede asignar a err, además de 0, es ERR_VAL_NOBOOLEAN.*/
bool8 lee_Bool(const char8_t **pc){
	bint b=0;
	while(**pc=='!') (*pc)++, b++;
	ifunlike(is_stn0(**pc)) return (bool8)(b&1);

	if(**pc=='S'){ ++*pc;
		if(**pc=='i') (*pc)++, b++;
		elifz(strbeginsby8(*pc,u8"í")) *pc+=strlen8(u8"í"), b++;
	}
	elif(**pc=='N' && *(++*pc)=='o') (*pc)++;

	return (bool8)(b&1);
}

//Lee la unidad y la almacena. Si no hay nada más en la línea será UNI_ABSENT
//Si la unidad no se reconoce será UNI_WRONG, en cuyo caso pc queda apuntando
//al principio de la palabra (e.d., no se avanza). En otro caso pc se avanza al final de la palabra.
void lee_unidad(const char8_t **pc, LongitudLeída *tl, enum TipoMagnitud typeM){
	tl->uni=UNI_ABSENT;
	if_Nomore_pc(*pc) return;
	if(**pc=='\0') return;
	switch(typeM){
	case Magnitud_Tabla:
		ifVALpc(u8"dmm") tl->uni=UNI_DMM, *pc+=3;
		elifVALpc(u8"mm") tl->uni=UNI_MM, *pc+=2;
		elifVALpc(u8"cm") tl->uni=UNI_CM, *pc+=2;
		elifVALpc(u8"m") tl->uni=UNI_M, (*pc)++;
		break;
	case Magnitud_TierraZ:
		ifVALpc(u8"cm") tl->uni=UNI_CM, *pc+=2;
		elifVALpc(u8"m") tl->uni=UNI_M, (*pc)++;
		break;
	case Magnitud_TierraPeq:
		ifVALpc(u8"cm") tl->uni=UNI_CM, *pc+=2;
		elifVALpc(u8"m") tl->uni=UNI_M, (*pc)++;
		elifVALpc(u8"\"") tl->uni=UNI_ARC_SEG, (*pc)++;
		elifVALpc(u8"\"'") tl->uni=UNI_ARC_TER, *pc+=2;
		elifVALpc(u8"'\"") tl->uni=UNI_ARC_TER, *pc+=2;
		elifVALpc(u8"dter") tl->uni=UNI_ARC_DTER, *pc+=4;
		elifVALpc(u8"mm") tl->uni=UNI_INVALID, *pc+=2;
		elif(isVALpc(u8"Km") || isVALpc(u8"km")) tl->uni=UNI_INVALID, *pc+=2;
		break;
	case Magnitud_TierraGrande:
		ifVALpc(u8"m") tl->uni=UNI_M, (*pc)++;
		elif(isVALpc(u8"Km") || isVALpc(u8"km")) tl->uni=UNI_KM, (*pc)+=2;
		elifVALpc(u8"\"") tl->uni=UNI_ARC_SEG, (*pc)++;
		elifVALpc(u8"°") tl->uni=UNI_ARC_GRADOS, (*pc)++;
		break;
	case Magnitud_Tiempo:
		ifVALpc(u8"ms") tl->uni=UNI_T_MSEC, *pc+=2;
		elifVALpc(u8"s") tl->uni=UNI_T_SEC, (*pc)++;
		elifVALpc(u8"m") tl->uni=UNI_T_MIN, (*pc)++;
		break;
	}
	if(tl->uni==UNI_ABSENT){
		if((**pc>='A' &&**pc<='Z') || (**pc>='a' && **pc<='z') || **pc>0x80)//primer byte de un carácter multi-byte
		   tl->uni=UNI_WRONG;
	}
}

/*Si el valor devuelto en .val es MAX_MAGNI_VAL es que el valor indicado en el fichero es demasiado grande.
Si las unidades no se reconocen o no hay, pc queda apuntando al principio de la palabra que
	hay allí o el final de línea si no hay nada tras el número.
Si las unidades se reconocen, pc queda apuntando past la unidad.

Se permite que existan decimales, pero se ignoran y se pondrá .res a MAGRES_ROUND.
El separador puede ser punto o coma.

pc se avanza al final de la unidad, salvo que se produzca algún error en cuyo caso queda donde el error.*/
LongitudLeída lee_Magnitud_no_decimales(iconst char8_t **pc, enum TipoMagnitud typeM){
	LongitudLeída tl;
	uint u;

	u=uint___str8(pc);
	if(u>=MAX_MAGNI_VAL){
		tl.val=MAX_MAGNI_VAL, tl.res=MAGRES_ROUND;
		return tl;
	}
	tl.val=u;

	tl.res=0;
	if(**pc=='.' || **pc==','){
		(*pc)++;
		if((char8_t)(**pc-'0')<10){tl.res=MAGRES_ROUND; do (*pc)++; while((char8_t)(**pc-'0')<10);}
	}
	Advanceinline_pc(*pc);
	lee_unidad(pc,&tl,typeM);
	return tl;
}

/*pc se avanza al final de la unidad, salvo que se produzca algún error en cuyo caso queda donde el error.*/
LongitudLeída lee_Magnitud_decimales(iconst char8_t **pc, enum TipoMagnitud typeM){
	LongitudLeída tl;
	uint u;
	u8int dec; //Número de cifras decimales leídas
	if(typeM==Magnitud_TierraGrande) return lee_Magnitud_no_decimales(pc,typeM);

	tl.uni=UNI_ABSENT;
	u=uint___str8(pc);

	tl.res=0;
	dec=0; //0 cifras decimales
	if(**pc=='.' || **pc==','){
		(*pc)++;
		if(u<MAX_MAGNI_VAL){
			uint c; //u no se hará >= MAX_MAGNI_VAL aquí dentro
			while((c=**pc-'0')<10){
				uint v=10*u+c;
				ifunlike(v>=MAX_MAGNI_VAL) break;
				(*pc)++, dec++;
				u=v;
			}
		}
	}
	ifunlike((char8_t)(**pc-'0')<10){
		tl.res=MAGRES_ROUND;
		do (*pc)++; while((char8_t)(**pc-'0')<10);
	}
	ifunlike(u>=MAX_MAGNI_VAL) tl.val=MAX_MAGNI_VAL, tl.res=MAGRES_ROUND;
	else tl.val=u;

	Advanceinline_pc(*pc);
	lee_unidad(pc,&tl,typeM);
	if(tl.uni==UNI_ABSENT || tl.uni>MAX_UNI_LONG){  //Normalmente será dec=0 aquí. Si se indican cifras decimales se indicarán
		ifunlike(dec){//seguramente también las unidades.
			do dec--, u/=10; while(dec); tl.val=u;
			tl.res=MAGRES_ROUND;
	}	}

	//Si en la lectura se pasó sobre una coma decimal, rebajar las unidades en la medida en que sea posible
	if(dec>0){//Hence uni!= ABSENT nor WRONG
		if(typeM==Magnitud_Tiempo){
			if(tl.uni==UNI_T_SEC){
				if(dec>=3) dec-=3, tl.uni=UNI_T_MSEC;
				elif(dec==1){
					if(100*tl.val<MAX_MAGNI_VAL) tl.val*=100, tl.uni=UNI_T_MSEC, dec=0;
				}else{ //dec=2
					if(10*tl.val<MAX_MAGNI_VAL) tl.val*=10, tl.uni=UNI_T_MSEC, dec=0;
				}
			}
		}else{
			if(tl.uni==UNI_M && dec==1){ //Se indicó resolución de dm, que se pasarán a cm si es posible.
				if(10*tl.val<MAX_MAGNI_VAL) tl.val*=10, tl.uni=UNI_CM, dec=0;
				//else, ya ser entrará abajo en dec!=0.
			}else if(typeM==Magnitud_Tabla){ //Bajar hasta dmm
				while(dec && tl.uni>UNI_DMM) dec--, tl.uni--;
			}else if(typeM==Magnitud_TierraZ){
				if(tl.uni==UNI_M) dec-=2, tl.uni=UNI_CM; //Bajar hasta cm
			}else if(typeM==Magnitud_TierraPeq){
				if(tl.uni==UNI_M) dec-=2, tl.uni=UNI_CM; //Bajar hasta cm
				elif(tl.uni==UNI_ARC_TER) dec--, tl.uni=UNI_ARC_DTER; //Bajar hasta dter
				//UNI_ARC_SEG no se puede bajar a una subdivisión decimal
			}
		}

		//Los decimales que no se hayan podido absorber con un cambio de unidades se ignoran.
		ifunlike(dec!=0){ //Hence no es tl.res=MAGRES_HUGE.
			tl.res=MAGRES_ROUND;
			do dec--, tl.val/=10; while(dec);
		}
	}

	return tl;
}

#undef isVALpc
#undef ifVALpc
#undef elifVALpc

//Puede cambiar err según el valor val comparardo con max_val y max_recommended.
sinline void check_Entero_range(int *err, uint val, uint max_val, uint max_recom){
	iflike(*err>=0){
		ifunlike(val>max_val) *err=ERR_VAL_RANGE;
		elifunlike(val>max_recom) *err=WAR_VAL_RANGE;
	}
}

//Puede cambiar err según el valor sval comparardo con bneg, max_val y max_recom.
//bneg indica si se permite que sval sea <0.
sinline void check_Magnitud_range(int *err, ssint sval, bint bneg, uint max_val, uint max_recom){
	iflike(*err>=0){
		ifunlike(sval<0 && !bneg){*err=ERR_VAL_NEG; return;}
		uint val=sval>=0 ? (uint)sval : (uint)-sval;
		ifunlike(val>max_val) *err=ERR_VAL_RANGE;
		elifunlike(val>max_recom) *err=WAR_VAL_RANGE;
	}
}

//Puede cambiar err según el valor med comparardo con max_val y max_recom.
sinline void check_uMagnitud_range(int *err, uint val, uint max_val, uint max_recom){
	iflike(*err>=0){
		ifunlike(val>max_val) *err=ERR_VAL_RANGE;
		elifunlike(val>max_recom) *err=WAR_VAL_RANGE;
	}
}

//Puede cambiar err según el valor sval comparardo con bneg, max_val y max_recom.
sinline void check_sMagnitud_range(int *err, ssint sval, uint max_val, uint max_recom){
	iflike(*err>=0){
		uint val=sval>=0 ? (uint)sval : (uint)-sval;
		ifunlike(val>max_val) *err=ERR_VAL_RANGE;
		elifunlike(val>max_recom) *err=WAR_VAL_RANGE;
	}
}

//val ha de ser >=0
//Puede cambiar err según el valor val comparardo con max_val y max_recommended.
sinline void check_uFloat_range(int *err, float val, float max_val, float max_recom){
	iflike(*err>=0){
		ifunlike(val>max_val) *err=ERR_VAL_RANGE;
		elifunlike(val>max_recom) *err=WAR_VAL_RANGE;
	}
}

//Puede cambiar err según el valor val comparardo con max_val y max_recommended.
sinline void check_sFloat_range(int *err, float sval, float max_val, float max_recom){
	iflike(*err>=0){
		float val=sval>=0 ? sval : -sval;
		ifunlike(val>max_val) *err=ERR_VAL_RANGE;
		elifunlike(val>max_recom) *err=WAR_VAL_RANGE;
	}
}
