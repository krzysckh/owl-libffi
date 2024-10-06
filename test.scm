(import
 (owl toplevel)
 (prefix (owl sys) sys/)
 (prefix (ffi) ffi/))

(define (deref-string ptr)
  (sys/mem-string
   (fold
    (λ (acc n) (bior (<< (bytevector-u8-ref ptr n) (* 8 n)) acc))
    0
    (iota 0 1 (bytevector-length ptr)))))

(λ (_)
  (define-values (malloc strcpy free)
    (ffi/funcs (ffi/open-library "libc.so")
      ("malloc" ffi/pointer ffi/uint32)
      ("strcpy" ffi/pointer ffi/pointer ffi/pointer)
      ("free"   ffi/void ffi/pointer)))

  (let* ((str (c-string "Hello, World!"))
         (ptr (malloc (string-length str))))
    (strcpy ptr str)
    (print (deref-string ptr))
    (free ptr))
  0)
