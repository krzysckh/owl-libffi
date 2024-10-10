a WIP ffi library for owl lisp

## DEPENDENCIES

- [libffi](https://sourceware.org/libffi/)
- owl lisp compiled from source (new enough to have FP_API)
- a c compiler

## USAGE

```scheme
(import
 (owl toplevel)
 (prefix (ffi) ffi/))

(λ (_)
  (define-values (c-puts c-open c-write c-close)
    (ffi/funcs (ffi/open-library "libc.so")
      ("puts" ffi/void ffi/pointer)                         ;; void puts(char*)
      ("open" ffi/int32 ffi/pointer ffi/int32)              ;; int open(char*, int)
      ("write" ffi/uint32 ffi/int32 ffi/pointer ffi/uint32) ;; uint write(int, void*, uint)
      ("close" ffi/int32 ffi/int32)))                       ;; int close(int)

  (c-puts "Hello, World!")
  (let ((fd (c-open "/dev/stdout" 2))
        (s "Hello, World from write(2)!\n"))
    (c-write fd s (string-length s))
    (c-close fd)))
```

then compile with

```shell
$ ol -x c -o file.c file.scm
$ cc -I/path/to/owl/source/c -DPRIM_FP_API -DPRIM_CUSTOM test.c ffi.c -lffi -ldl -o test
```

## FILES

- `ffi.c` - the module code that gets injected into the vm that interacts with libffi and libdl
- `ffi.scm` - a user-friendly lisp front-end to `ffi.c`

## CAVEATS

- probably won't run under owl-winrt because of the `dlopen` and `dlsym` calls
  (might get ported some day)

this might be transformed into a [extension](https://gitlab.com/owl-lisp/owl/-/tree/master/ext?ref_type=heads)
if it turns out that they will stay in owl for good.
