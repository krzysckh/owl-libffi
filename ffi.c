#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ffi.h>
#include <dlfcn.h>
#include <err.h>

#ifndef PRIM_FP_API
#define PRIM_FP_API
#endif

#include "ovm.h"
#undef assert
#include <assert.h>

word
thing_to_word(void *p, ffi_type *type, int *freep)
{
  *freep = 1;
  if (type == &ffi_type_void)   return INULL;
  if (type == &ffi_type_uint8)  return onum(*(uint8_t*)p, 0);
  if (type == &ffi_type_sint8)  return onum(*(int8_t*)p, 1);
  if (type == &ffi_type_uint16) return onum(*(uint16_t*)p, 0);
  if (type == &ffi_type_sint16) return onum(*(int16_t*)p, 1);
  if (type == &ffi_type_uint32) return onum(*(uint32_t*)p, 0);
  if (type == &ffi_type_sint32) return onum(*(int32_t*)p, 1);
  if (type == &ffi_type_uint64) return onum(*(uint64_t*)p, 0);
  if (type == &ffi_type_sint64) return onum(*(int64_t*)p, 1);
  if (type == &ffi_type_float)  return mkrat_approx(*(float*)p);
  if (type == &ffi_type_double) return mkrat_approx(*(double*)p);
  if (type == &ffi_type_pointer) {
    void *ptr = *(void**)p;
    return PTR(ptr);
  }

  *freep = 0;
  return PTR(p);
}

void *
word_to_thing(word w, ffi_type *type)
{
  if (type == &ffi_type_uint8 || type == &ffi_type_sint8) {
    uint8_t *v = malloc(sizeof(uint8_t));
    *v = cnum(w);
    return v;
  } else if (type == &ffi_type_uint16 || type == &ffi_type_sint16) {
    uint16_t *v = malloc(sizeof(uint16_t));
    *v = cnum(w);
    return v;
  } else if (type == &ffi_type_uint32 || type == &ffi_type_sint32) {
    uint32_t *v = malloc(sizeof(uint32_t));
    *v = cnum(w);
    return v;
  } else if (type == &ffi_type_uint64 || type == &ffi_type_sint64) {
    uint64_t *v = malloc(sizeof(uint64_t));
    *v = cnum(w);
    return v;
  } else if (type == &ffi_type_float) {
    float *v = malloc(sizeof(float));
    *v = (float)cdouble(w);
    return v;
  } else if (type == &ffi_type_double) {
    double *v = malloc(sizeof(double));
    *v = cdouble(w);
    return v;
  } else if (type == &ffi_type_pointer) {
    void **v = malloc(sizeof(void*));
    if (stringp(w))
      *v = cstr(w);
    else
      *v = cptr(w);
    return v;
  } else {
    assert(type->type == FFI_TYPE_STRUCT);
    void *v = malloc(type->size);
    memcpy(v, cptr(w), type->size);
    return v;
  }
}

ffi_type*
type_ptr(word w)
{
  ffi_type *types[] = {
    &ffi_type_void,
    &ffi_type_uint8,
    &ffi_type_sint8,
    &ffi_type_uint16,
    &ffi_type_sint16,
    &ffi_type_uint32,
    &ffi_type_sint32,
    &ffi_type_uint64,
    &ffi_type_sint64,
    &ffi_type_float,
    &ffi_type_double,
    &ffi_type_pointer,
  };

  if (is_type(w, TNUM))
    return types[cnum(w)];
  return cptr(w);
}

word
prim_custom(int op, word a, word b, word c)
{

  switch (op) {
  case 300: { // prep-cif (arg-type1 ... arg-typeN) ret-type
    uint arity = llen((word*)a), i;
    ffi_type *ret = type_ptr(b), **args = malloc(sizeof(ffi_type*) * arity);
    ffi_cif *cif = malloc(sizeof(ffi_cif));

    for (i = 0; a != INULL; i++, a = cdr(a))
      args[i] = type_ptr(car(a));

    // TODO: can i free args?
    if (ffi_prep_cif(cif, FFI_DEFAULT_ABI, arity, ret, args) == FFI_OK)
      return PTR(cif);
    return IFALSE;
  }
  case 301: { // dlopen lib → lib-ptr
    void *p = dlopen(cstr(a), RTLD_NOW);
    return PTR(p);
  }
  case 302: { // dlsym lib-ptr sym → fn-ptr
    void *p = dlsym(cptr(a), cstr(b));
    return PTR(p);
  }
  case 303: { // call fn cif (arg ... argN) → retv
    uint arity = llen((word*)c), i;
    ffi_cif *cif = cptr(b);
    void **values = malloc(sizeof(void*) * arity);
    void *rc = malloc(sizeof(double));
    int freep = 0;

    // this seems ugly
    for (i = 0; c != INULL; ++i, c = cdr(c)) {
      values[i] = word_to_thing(car(c), cif->arg_types[i]);
    }

    ffi_call(cif, cptr(a), rc, values);

    for (i = 0; i < arity; ++i)
      free(values[i]);

    word w = thing_to_word(rc, cif->rtype, &freep);
    if (freep)
      free(rc);

    return w;
  }
  case 304: { // defstruct (T1 ... Tn)
    uint i = 0;
    ffi_type *t = malloc(sizeof(ffi_type)), *tl[1];
    ffi_cif temp_cif;
    t->size = t->alignment = 0;
    t->type = FFI_TYPE_STRUCT;
    t->elements = malloc(sizeof(ffi_type*) * (llen((word*)a) + 1));
    for (; a != INULL; ++i, a = cdr(a))
      t->elements[i] = type_ptr(car(a));
    t->elements[i] = NULL;

    tl[0] = t;

    if (ffi_prep_cif(&temp_cif, FFI_DEFAULT_ABI, 1, &ffi_type_void, tl) == FFI_OK)
      return PTR(t);

    return IFALSE;
  }
  case 305: { // mkstruct ptr (v1 ... vn) → ptr
    ffi_type *t = cptr(a);
    uint i = 0;
    void *retv = malloc(t->size), *rp = retv, *v;

    for (; b != INULL; b = cdr(b), ++i) {
      v = word_to_thing(car(b), t->elements[i]);
      memcpy(rp, v, t->elements[i]->size);
      rp += t->elements[i]->size;
    }

    return PTR(retv);
  }
  }

  return IFALSE;
}
