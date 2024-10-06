#include <stdio.h>
#include <stdlib.h>
#include <ffi.h>
#include <dlfcn.h>
#include <err.h>

#ifndef PRIM_FP_API
#define PRIM_FP_API
#endif

#include "ovm.h"

word
prim_custom(int op, word a, word b, word c)
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

  switch (op) {
  case 300: { // prep-cif (arg-type1 ... arg-typeN) ret-type
    uint arity = llen((word*)a), i;
    ffi_type *ret = types[cnum(b)], **args = malloc(sizeof(ffi_type*) * arity);
    ffi_cif *cif = malloc(sizeof(ffi_cif));

    for (i = 0; a != INULL; i++, a = cdr(a))
      args[i] = types[cnum(car(a))];

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

    // this seems ugly
    for (i = 0; c != INULL; ++i, c = cdr(c)) {
      if (cif->arg_types[i] == &ffi_type_uint8 || cif->arg_types[i] == &ffi_type_sint8) {
        uint8_t *v = malloc(sizeof(uint8_t));
        *v = cnum(car(c));
        values[i] = v;
      } else if (cif->arg_types[i] == &ffi_type_uint16 || cif->arg_types[i] == &ffi_type_sint16) {
        uint16_t *v = malloc(sizeof(uint16_t));
        *v = cnum(car(c));
        values[i] = v;
      } else if (cif->arg_types[i] == &ffi_type_uint32 || cif->arg_types[i] == &ffi_type_sint32) {
        uint32_t *v = malloc(sizeof(uint32_t));
        *v = cnum(car(c));
        values[i] = v;
      } else if (cif->arg_types[i] == &ffi_type_uint64 || cif->arg_types[i] == &ffi_type_sint64) {
        uint64_t *v = malloc(sizeof(uint64_t));
        *v = cnum(car(c));
        values[i] = v;
      } else if (cif->arg_types[i] == &ffi_type_float) {
        float *v = malloc(sizeof(float));
        *v = (float)cdouble(car(c));
        values[i] = v;
      } else if (cif->arg_types[i] == &ffi_type_double) {
        double *v = malloc(sizeof(double));
        *v = cdouble(car(c));
        values[i] = v;
      } else if (cif->arg_types[i] == &ffi_type_pointer) {
        void **v = malloc(sizeof(void*));
        if (stringp(car(c)))
          *v = cstr(car(c));
        else
          *v = cptr(car(c));
        values[i] = v;
      } else {
        errx(1, "unsupported type passed");
      }
    }

    ffi_call(cif, cptr(a), rc, values);

    for (i = 0; i < arity; ++i)
      free(values[i]);

    // is this needed? can't i just cast to the biggest value possible?
    if (cif->rtype == &ffi_type_void)   return INULL;
    if (cif->rtype == &ffi_type_uint8)  return onum(*(uint8_t*)rc, 0);
    if (cif->rtype == &ffi_type_sint8)  return onum(*(int8_t*)rc, 1);
    if (cif->rtype == &ffi_type_uint16) return onum(*(uint16_t*)rc, 0);
    if (cif->rtype == &ffi_type_sint16) return onum(*(int16_t*)rc, 1);
    if (cif->rtype == &ffi_type_uint32) return onum(*(uint32_t*)rc, 0);
    if (cif->rtype == &ffi_type_sint32) return onum(*(int32_t*)rc, 1);
    if (cif->rtype == &ffi_type_uint64) return onum(*(uint64_t*)rc, 0);
    if (cif->rtype == &ffi_type_sint64) return onum(*(int64_t*)rc, 1);
    if (cif->rtype == &ffi_type_float)  return mkrat_approx(*(float*)rc);
    if (cif->rtype == &ffi_type_double) return mkrat_approx(*(double*)rc);
    if (cif->rtype == &ffi_type_pointer) {
      void *p = *(void**)rc;
      return PTR(p);
    }
    return INULL;
  }
  }

  return IFALSE;
}
