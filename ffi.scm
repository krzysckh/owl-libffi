(define-library (ffi)
  (import
   (owl toplevel)
   )

  (export
   void uint8 sint8 int8
   uint16 sint16 int16 uint32
   sint32 int32 uint64 sint64
   int64 float double pointer

   prep-cif
   dlopen
   open-library
   dlsym
   call

   defstruct*
   mkstruct*
   defstruct ;; use this one probably

   func
   funcs
   )

  (begin
    ;; types stored as a list in prim_custom
    (define void 0)
    (define uint8 1)
    (define sint8 2)
    (define int8 sint8)
    (define uint16 3)
    (define sint16 4)
    (define int16 sint16)
    (define uint32 5)
    (define sint32 6)
    (define int32 sint32)
    (define uint64 7)
    (define sint64 8)
    (define int64 sint64)
    (define float 9)
    (define double 10)
    (define pointer 11)

    (define (unfuck-strings xs)
      (map (λ (x) (if (string? x) (c-string x) x)) xs))

    ;; prepare libffi arg cif
    (define (prep-cif types ret)
      (sys-prim 300 types ret #f))

    ;; open a dynamic shared library
    (define (dlopen f)
      (sys-prim 301 (c-string f) #f #f))

    (define open-library dlopen)

    ;; find a symbol in a opened library
    (define (dlsym lib sym)
      (sys-prim 302 lib (c-string sym) #f))

    ;; call fn with cif and (args)
    (define (call cif fn args)
      (sys-prim 303 fn cif (unfuck-strings args)))

    (define (defstruct* types)
      (sys-prim 304 types #f #f))

    (define (mkstruct* struct xs)
      (sys-prim 305 struct (unfuck-strings xs) #f))

    ;; (type1 ... typeN) → (values struct-data-ptr make-struct)
    (define (defstruct . types)
      (let ((struct (defstruct* types)))
        (values
         struct
         (λ xs
           (mkstruct* struct (unfuck-strings xs))))))

    ;; create a c-function func with library lib, name func-name, return type ret and arg types T ...
    (define-syntax func
      (syntax-rules ()
        ((func lib func-name ret T ...)
         (let ((cif (prep-cif (list T ...) ret))
               (fn (dlsym lib func-name)))
           (λ args
             (call cif fn args))))))

    ;; create many functions from library lib and return them in (values)
    (define-syntax funcs
      (syntax-rules ()
        ((funcs _lib (func-name ret T ...) ...)
         (let ((lib (if (bytevector? _lib) _lib (open-library _lib))))
           (values
            (let ((cif (prep-cif (list T ...) ret))
                  (fn (dlsym lib func-name)))
              (λ args (call cif fn args))) ...)))))
    ))
