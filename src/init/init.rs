
#![no_std]

mod mbi;

use core::panic::PanicInfo;
use core::arch::asm;
use mbi::*;

extern "C"
{
        fn PDEInit();
        fn gdtInit();
        fn idtInit();
        fn pitInit(freq: u32);
        fn memInit(total_mem: u32);
        fn schedInit();
        fn SerialInit();
        fn init_parallel_ports();
        fn FeaturesInit();
        fn ps2_keyboard_present() -> i32;
        fn ps2_mouse_present() -> i32;
        fn ps2_initialize_mouse();
        fn ps2_initialize_keyboard();
        fn ps2_getchar() -> u8;
        fn pciEnumerateDevices(callback: extern "C" fn());
        fn pciRegister();
        fn SMInit();
        fn SMChange(drive: i32);
        fn SMGetDrive() -> *mut u8;
        fn FatTest(drive: *mut u8);
        fn malloc(size: usize) -> *mut u8;
        fn free(ptr: *mut u8);
        fn printf(format: *const u8, ...);
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> !
{
        loop {}
}

#[inline(always)]
pub unsafe fn hlt()
{
        asm!("hlt", options(nomem, nostack));
}

#[no_mangle]
fn rmain(magic: u32, mem_lower: u32, mem_upper: u32)
{
        if magic != 0x2BADB002
        {
                loop {}
        }
        unsafe
        {
                PDEInit();
                FeaturesInit();
                let total = (mem_upper as u32).wrapping_mul(1024)
                           .wrapping_add((mem_lower as u32).wrapping_mul(1024));
                gdtInit();
                idtInit();
                pitInit(250);
                memInit(total);
                schedInit();
                SerialInit();
                init_parallel_ports();
        }
}
