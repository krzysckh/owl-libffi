(import
 (owl toplevel)
 (prefix (ffi) ffi/))

(define (fbool f) (λ () (if (= 0 (f)) #f #t)))

(λ (_)
  (lets ((Color make-Color (ffi/defstruct ffi/uint8 ffi/uint8 ffi/uint8 ffi/uint8))
         (raywhite (make-Color 245 245 245 255))
         (gray-ish (make-Color 200 200 200 255))
         (init-window
          set-target-fps!
          begin-drawing
          clear-background
          draw-text
          end-drawing
          close-window
          _win-should-close?
          (ffi/funcs (ffi/open-library "libraylib.so")
           ("InitWindow" ffi/void ffi/int32 ffi/int32 ffi/pointer)
           ("SetTargetFPS" ffi/void ffi/int32)
           ("BeginDrawing" ffi/void)
           ("ClearBackground" ffi/void Color)
           ("DrawText" ffi/void ffi/pointer ffi/int32 ffi/int32 ffi/int32 Color)
           ("EndDrawing" ffi/void)
           ("CloseWindow" ffi/void)
           ("WindowShouldClose" ffi/uint8)))
         (win-should-close? (fbool _win-should-close?)))
    (init-window 800 450 "sigmon [core] example - basic window")
    (set-target-fps! 60)
    (let loop ()
      (begin-drawing)
      (clear-background raywhite)
      (draw-text "Congrats! You created your first window!" 190 200 20 gray-ish)
      (end-drawing)
      (if (win-should-close?)
          0
          (loop)))))
