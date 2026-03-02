#![no_std]

use core::panic::PanicInfo;

extern "C"
{
        fn SerialPrint(str: *const u8);
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> !
{
        loop {}
}

#[no_mangle]
fn rmain()
{
        let message = "Hello from Rust!\n";
        unsafe
        {
                SerialPrint(message.as_ptr());
        }
}
