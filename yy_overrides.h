//Header include provides replacements for default flex memory allocator
//  Scanner must NOT be reentrant for this to work!

extern int yylineno;

void* yyalloc(size_t bytes) {
  //Tell collector to look inside the allocation for pointers
  //  but do not handle collection of the memory itself because
  //  we dont want to assume flex-bison never obscures pointers
  return GC_MALLOC_UNCOLLECTABLE(bytes);
}

void* yyrealloc(void* ptr, size_t bytes) {
  return GC_REALLOC(ptr,bytes);
}

void yyfree(void* ptr) {
  GC_FREE(ptr);
}

void yyerror(const char* s) {
  printf("Error in parser near line %d: %s\n", yylineno, s);
}

void pprintf(const char* s) {
 #ifdef YYDEBUG
   printf(s);
 #else
   return;
 #endif
}

void pprintf(const char* s, const char* a) {
 #ifdef YYDEBUG
   printf(s,a);
 #else
   return;
 #endif
}

void pprintf(const char* s, const int a) {
 #ifdef YYDEBUG
   printf(s,a);
 #else
   return;
 #endif
}

//Special injection of special externs
std::vector<Statement*,gc_allocator<Statement*>>* buildInjections() {
	auto injections = new std::vector<Statement*,gc_allocator<Statement*>>();
	char* cvoid = (char*)GC_MALLOC_ATOMIC(8);  //mutable strings
	char* cint = (char*)GC_MALLOC_ATOMIC(8);
	char* cfloat = (char*)GC_MALLOC_ATOMIC(8);
	char* c__make_context = (char*)GC_MALLOC_ATOMIC(32);
	char* c__fork_sched_int = (char*)GC_MALLOC_ATOMIC(32);
	char* c__fork_sched_float = (char*)GC_MALLOC_ATOMIC(32);
	char* c__fork_sched_intptr = (char*)GC_MALLOC_ATOMIC(32);
	char* c__fork_sched_floatptr = (char*)GC_MALLOC_ATOMIC(32);
	char* c__fork_sched_void = (char*)GC_MALLOC_ATOMIC(32);
	char* c__recon_int = (char*)GC_MALLOC_ATOMIC(32);
	char* c__recon_float = (char*)GC_MALLOC_ATOMIC(32);
	char* c__recon_intptr = (char*)GC_MALLOC_ATOMIC(32);
	char* c__recon_floatptr = (char*)GC_MALLOC_ATOMIC(32);
	char* c__recon_void = (char*)GC_MALLOC_ATOMIC(32);
	char* c__destroy_context = (char*)GC_MALLOC_ATOMIC(32);
	char* c_func = (char*)GC_MALLOC_ATOMIC(8);
	char* c_env = (char*)GC_MALLOC_ATOMIC(8);
	char* c_id = (char*)GC_MALLOC_ATOMIC(8);
	char* c_cid = (char*)GC_MALLOC_ATOMIC(8);
	char* c_original = (char*)GC_MALLOC_ATOMIC(32);
	char* c_known = (char*)GC_MALLOC_ATOMIC(8);
	char* c_max = (char*)GC_MALLOC_ATOMIC(8);
	char* cmalloc_int = (char*)GC_MALLOC_ATOMIC(32);
	char* cmalloc_float = (char*)GC_MALLOC_ATOMIC(32);
	char* ccalloc_int = (char*)GC_MALLOC_ATOMIC(32);
	char* ccalloc_float = (char*)GC_MALLOC_ATOMIC(32);
	char* cfree_int = (char*)GC_MALLOC_ATOMIC(32);
	char* cfree_float = (char*)GC_MALLOC_ATOMIC(32);
	std::strcpy(cvoid,"void");
	std::strcpy(cint,"int");
	std::strcpy(cfloat,"float");
	std::strcpy(cmalloc_int,"malloc_int");
	std::strcpy(cmalloc_float,"malloc_float");
	std::strcpy(ccalloc_int,"calloc_int");
	std::strcpy(ccalloc_float,"calloc_float");
	std::strcpy(cfree_int,"free_int");
	std::strcpy(cfree_float,"free_float");
	std::strcpy(c__make_context,"__make_context");
	std::strcpy(c__destroy_context,"__destroy_context");
	std::strcpy(c__fork_sched_int,"__fork_sched_int");
	std::strcpy(c__fork_sched_float,"__fork_sched_float");
	std::strcpy(c__fork_sched_intptr,"__fork_sched_intptr");
	std::strcpy(c__fork_sched_floatptr,"__fork_sched_floatptr");
	std::strcpy(c__fork_sched_void,"__fork_sched_void");
	std::strcpy(c__recon_int,"__recon_int");
	std::strcpy(c__recon_float,"__recon_float");
	std::strcpy(c__recon_intptr,"__recon_intptr");
	std::strcpy(c__recon_floatptr,"__recon_floatptr");
	std::strcpy(c__recon_void,"__recon_void");
	std::strcpy(c_func,"func");
	std::strcpy(c_env,"env");
	std::strcpy(c_id,"id");
	std::strcpy(c_cid,"cid");
	std::strcpy(c_original,"original");
	std::strcpy(c_known,"known");
	std::strcpy(c_max,"max");
	Keyword* kvoid = new Keyword(cvoid); //Keywords
	Keyword* kint = new Keyword(cint);
	Keyword* kfloat = new Keyword(cfloat);
	Identifier* i__make_context = new Identifier(c__make_context); //Identifiers
	Identifier* i__fork_sched_int = new Identifier(c__fork_sched_int);
	Identifier* i__fork_sched_float = new Identifier(c__fork_sched_float);
	Identifier* i__fork_sched_intptr = new Identifier(c__fork_sched_intptr);
	Identifier* i__fork_sched_floatptr = new Identifier(c__fork_sched_floatptr);
	Identifier* i__fork_sched_void = new Identifier(c__fork_sched_void);
	Identifier* i__recon_int = new Identifier(c__recon_int);
	Identifier* i__recon_float = new Identifier(c__recon_float);
	Identifier* i__recon_intptr = new Identifier(c__recon_intptr);
	Identifier* i__recon_floatptr = new Identifier(c__recon_floatptr);
	Identifier* i__recon_void = new Identifier(c__recon_void);
	Identifier* i__destroy_context = new Identifier(c__destroy_context);
	Identifier* imalloc_int = new Identifier(cmalloc_int);
	Identifier* imalloc_float = new Identifier(cmalloc_float);
	Identifier* icalloc_int = new Identifier(ccalloc_int);
	Identifier* icalloc_float = new Identifier(ccalloc_float);
	Identifier* ifree_int = new Identifier(cfree_int);
	Identifier* ifree_float = new Identifier(cfree_float);
	VariableDefinition* vfunc = new VariableDefinition(kvoid,new Identifier(c_func),nullptr,true); //Variables
	VariableDefinition* venv = new VariableDefinition(kvoid,new Identifier(c_env),nullptr,true);
	VariableDefinition* vid = new VariableDefinition(kint,new Identifier(c_id),nullptr,false);
	VariableDefinition* vcid = new VariableDefinition(kint,new Identifier(c_cid),nullptr,false);
	VariableDefinition* vmax = new VariableDefinition(kint,new Identifier(c_max),nullptr,false);
	VariableDefinition* vknownint = new VariableDefinition(kint,new Identifier(c_known),nullptr,false);
	VariableDefinition* vknownintptr = new VariableDefinition(kint,new Identifier(c_known),nullptr,true);
	VariableDefinition* vknownfloat = new VariableDefinition(kfloat,new Identifier(c_known),nullptr,false);
	VariableDefinition* vknownfloatptr = new VariableDefinition(kfloat,new Identifier(c_known),nullptr,true);
	VariableDefinition* voriginalint = new VariableDefinition(kint,new Identifier(c_original),nullptr,false);
	VariableDefinition* voriginalintptr = new VariableDefinition(kint,new Identifier(c_original),nullptr,true);
	VariableDefinition* voriginalfloat = new VariableDefinition(kfloat,new Identifier(c_original),nullptr,false);
	VariableDefinition* voriginalfloatptr = new VariableDefinition(kfloat,new Identifier(c_original),nullptr,true);

	auto v__make_context = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>(); //Call vectors
	auto vmalloc_int = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
	vmalloc_int->push_back(vid);
	auto vmalloc_float = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
	vmalloc_float->push_back(vid);
	auto vcalloc_int = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
	vcalloc_int->push_back(vid);
	auto vcalloc_float = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
	vcalloc_float->push_back(vid);
	auto vfree_int = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
	vfree_int->push_back(voriginalintptr);
	auto vfree_float = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
	vfree_float->push_back(voriginalfloatptr);
	auto v__fork_sched_int = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
	v__fork_sched_int->push_back(vfunc);
	v__fork_sched_int->push_back(venv);
	v__fork_sched_int->push_back(vid);
	v__fork_sched_int->push_back(vcid);
	auto v__fork_sched_float = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
	v__fork_sched_float->push_back(vfunc);
	v__fork_sched_float->push_back(venv);
	v__fork_sched_float->push_back(vid);
	v__fork_sched_float->push_back(vcid);
	auto v__fork_sched_intptr = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
	v__fork_sched_intptr->push_back(vfunc);
	v__fork_sched_intptr->push_back(venv);
	v__fork_sched_intptr->push_back(vid);
	v__fork_sched_intptr->push_back(vcid);
	auto v__fork_sched_floatptr = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
	v__fork_sched_floatptr->push_back(vfunc);
	v__fork_sched_floatptr->push_back(venv);
	v__fork_sched_floatptr->push_back(vid);
	v__fork_sched_floatptr->push_back(vcid);
	auto v__fork_sched_void = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
	v__fork_sched_void->push_back(vfunc);
	v__fork_sched_void->push_back(venv);
	v__fork_sched_void->push_back(vid);
	v__fork_sched_void->push_back(vcid);
	auto v__recon_int = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
	v__recon_int->push_back(voriginalint);
	v__recon_int->push_back(vknownint);
	v__recon_int->push_back(vid);
	v__recon_int->push_back(vmax);
	v__recon_int->push_back(vcid);
	auto v__recon_float = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
	v__recon_float->push_back(voriginalfloat);
	v__recon_float->push_back(vknownfloat);
	v__recon_float->push_back(vid);
	v__recon_float->push_back(vmax);
	v__recon_float->push_back(vcid);
	auto v__recon_intptr = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
	v__recon_intptr->push_back(voriginalintptr);
	v__recon_intptr->push_back(vknownintptr);
	v__recon_intptr->push_back(vid);
	v__recon_intptr->push_back(vmax);
	v__recon_intptr->push_back(vcid);
	auto v__recon_floatptr = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
	v__recon_floatptr->push_back(voriginalfloatptr);
	v__recon_floatptr->push_back(vknownfloatptr);
	v__recon_floatptr->push_back(vid);
	v__recon_floatptr->push_back(vmax);
	v__recon_floatptr->push_back(vcid);
	auto v__recon_void = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
	v__recon_void->push_back(vid);
	v__recon_void->push_back(vmax);
	v__recon_void->push_back(vcid);
	auto v__destroy_context = new std::vector<VariableDefinition*,gc_allocator<VariableDefinition*>>();
	v__destroy_context->push_back(vcid);
	injections->push_back(new ExternStatement(kint,imalloc_int,vmalloc_int,true,true));
	injections->push_back(new ExternStatement(kfloat,imalloc_float,vmalloc_float,true,true));
	injections->push_back(new ExternStatement(kint,icalloc_int,vcalloc_int,true,true));
	injections->push_back(new ExternStatement(kfloat,icalloc_float,vcalloc_float,true,true));
	injections->push_back(new ExternStatement(kvoid,ifree_int,vfree_int,false,true));
	injections->push_back(new ExternStatement(kvoid,ifree_float,vfree_float,false,true));
	injections->push_back(new ExternStatement(kint,i__make_context,v__make_context,false,true));
	injections->push_back(new ExternStatement(kvoid,i__fork_sched_int,v__fork_sched_int,false,true));
	injections->push_back(new ExternStatement(kvoid,i__fork_sched_float,v__fork_sched_float,false,true));
	injections->push_back(new ExternStatement(kvoid,i__fork_sched_intptr,v__fork_sched_intptr,false,true));
	injections->push_back(new ExternStatement(kvoid,i__fork_sched_floatptr,v__fork_sched_floatptr,false,true));
	injections->push_back(new ExternStatement(kvoid,i__fork_sched_void,v__fork_sched_void,false,true));
	injections->push_back(new ExternStatement(kint,i__recon_int,v__recon_int,false,true));
	injections->push_back(new ExternStatement(kfloat,i__recon_float,v__recon_float,false,true));
	injections->push_back(new ExternStatement(kint,i__recon_intptr,v__recon_intptr,true,true));
	injections->push_back(new ExternStatement(kfloat,i__recon_floatptr,v__recon_floatptr,true,true));
	injections->push_back(new ExternStatement(kvoid,i__recon_void,v__recon_void,false,true));
	injections->push_back(new ExternStatement(kvoid,i__destroy_context,v__destroy_context,false,true));
	return injections;
}


