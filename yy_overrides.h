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
