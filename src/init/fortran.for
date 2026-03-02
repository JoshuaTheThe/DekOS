subroutine fmain() bind(C)
        implicit none
        ! Declare external C function
        interface
                subroutine printf(str) bind(C)
                        import
                        character, dimension(*), intent(in) :: str
                end subroutine printf
        end interface
        
        character(len=29) :: msg
        msg = " [INFO] Hello from Fortran!" // char(10) // char(0)
        
        call printf(msg)
end subroutine fmain