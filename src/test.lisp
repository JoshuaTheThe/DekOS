(fn LispTest (
        (extern printf)
        (fn foo n (
                (if (gt n 0) (
                        (foo (sub n 1))
                        (printf "Hello, World, From LISP! %d\n" n)
                ))
        ))

        (foo 10)
))