/* KV_BeginExpand y KV_EndExpand nunca están encajados. De las funciones de este fichero
y de expand_bufline.c, las únicas que pueden encontrarlos son reexpand() y expand_Γ. Porque:
	* Las de expand_bufline expanden texto tal cual encontrado en el fichero
	* Solamente puede surgir:
		- Al copiar o expandir según las reglas el texto de una variable y releerlo. Esto se hace,
		o bien para parsearlo como un cierto tipo, cuyo código está en otra parte, o en
		expand_Γ para el contenido del primer [ ... ].
		- Al expandir el texto de una variable. Todas pasan por reexpand().
	* reexpand() detecta los tramos entre KV_BE y KV_EE, los aísla '\0'-ended y los remite
		a expand_text_0.
*/

/*Devuelve el MacObj de la variable de nombre pc. pc is '\0'-ended
Si no existe la crea.
Si devuelve NULL es que se intentó crear pero no hay memoria
Si existe pero no es de tipo MACRO_OBJECT la elimina de donde estuviera y la añade entre los objetos. */
static MacObj *findcreate_obj_0(KeyVals *kvs, const char8_t *pc){
	uint keykey=Я;
	hBase8WhereKey *w=geth_hBase8WhereKey(&kvs->hash,pc);
	if(w!=NULL){
		if(w->data.macro_type==MACRO_OBJECT) return kvs->objs.ppio+w->data.n;
		if(w->data.macro_type==MACRO_FUNC){
			MacFun *kv=kvs->funcs.ppio+w->data.n;
			keykey=kv->c.key; //Recordar el nombre
			MacFun_defaults(*kv);
		}
	}

	MacObj kv;
	MacObj_defaults(kv);
	if(keykey!=Я) kv.c.key=keykey;
	else{
		kv.c.key=(pdif)(kvs->strs.next-kvs->strs.ppio);
		GC8_adds0(kvs->strs,pc,return NULL);
	}
	Vadd(kvs->objs,MacObj,kv,return NULL);
	//
	WhereKey wk={.macro_type=MACRO_OBJECT, .n=kvs->objs.n-1};
	if(w!=NULL) w->data=wk;
	else addh_keydata(hBase8WhereKey,&kvs->hash,kv.c.key,wk,return NULL);

	return kvs->objs.ppio+kvs->objs.n-1;
}

/*Devuelve el MacFun de la variable de nombre pc. pc is '\0'-ended
Si no existe la crea.
Si devuelve NULL es que se intentó crear pero no hay memoria
Si existe pero no es de tipo MACRO_FUNC, la elimina de donde estuviera y la añade entre las funciones. */
static MacFun *findcreate_func_0(KeyVals *kvs, const char8_t *pc){
	uint keykey=Я;
	hBase8WhereKey *w=geth_hBase8WhereKey(&kvs->hash,pc);
	if(w!=NULL){
		if(w->data.macro_type==MACRO_FUNC) return kvs->funcs.ppio+w->data.n;
		if(w->data.macro_type==MACRO_OBJECT){
			MacObj *kv=kvs->objs.ppio+w->data.n;
			keykey=kv->c.key; //Recordar el nombre
			MacObj_defaults(*kv);
		}
	}

	MacFun kv;
	MacFun_defaults(kv);
	if(keykey!=Я) kv.c.key=keykey;
	else{
		kv.c.key=(pdif)(kvs->strs.next-kvs->strs.ppio);
		GC8_adds0(kvs->strs,pc,return NULL);
	}
	Vadd(kvs->funcs,MacFun,kv,return NULL);
	//
	WhereKey wk={.macro_type=MACRO_FUNC, .n=kvs->funcs.n-1};
	if(w!=NULL) w->data=wk;
	else addh_keydata(hBase8WhereKey,&kvs->hash,kv.c.key,wk,return NULL);

	return kvs->funcs.ppio+kvs->funcs.n-1;
}

/*Añade kv a kvs->hash. Solamente el objeto. No añade a kvs->strs los strings asociados.
Si ya está y el el tipo del que ya hay es el mismo que el de kv, copia el objeto kv
	pasado sobre el kv que ya hay, en kvs->objs o kvs->funcs.
Si ya está y es de otro tipo, primero pone a defaults el que ya está en kvs->objs
	o kvs->funcs, luego añade kv al array correspondiente y modifica la entrada
	de la hash para que apunte al añadido.
*/
static int add_to_hash(KeyVals *kvs, const KeyVal kv){
	MacroCommon *com;
	if(kv.macro_type==MACRO_OBJECT) com=&kv.p.obj->c;
	elif(kv.macro_type==MACRO_FUNC) com=&kv.p.fun->c;
	else return 0;

	hBase8WhereKey *w=geth_hBase8WhereKey(&kvs->hash,kvs->strs.ppio+com->key);
	if(w!=NULL && w->data.macro_type==kv.macro_type){
		if(w->data.macro_type==MACRO_OBJECT) kvs->objs.ppio[w->data.n]=*kv.p.obj;
		elif(w->data.macro_type==MACRO_FUNC) kvs->funcs.ppio[w->data.n]=*kv.p.fun;
		else assert("Shoud not get here");
		return 0;
	}

	uint n;
	if(kv.macro_type==MACRO_OBJECT){
		n=kvs->objs.n;
		Vadd(kvs->objs,MacObj,*kv.p.obj,return AT_NOMEM);
	}else{
		n=kvs->funcs.n;
		Vadd(kvs->funcs,MacFun,*kv.p.fun,return AT_NOMEM);
	}
	if(w!=NULL){
		if(w->data.macro_type==MACRO_OBJECT){MacObj_defaults(kvs->objs.ppio[w->data.n]);}
		elif(w->data.macro_type==MACRO_FUNC){MacFun_defaults(kvs->funcs.ppio[w->data.n]);}
		w->data.macro_type=kv.macro_type;
		w->data.n=n;
	}else{
		WhereKey wk={.macro_type=kv.macro_type, .n=n};
		addh_keydata(hBase8WhereKey,&kvs->hash,com->key,wk,return AT_NOMEM);
	}

	return 0;
}
