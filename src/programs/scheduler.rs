
#![no_std]

use core::panic::PanicInfo;
use core::arch::asm;
use core::ffi::c_void;

#[panic_handler]
fn panic(_info: &PanicInfo) -> !
{
        loop {}
}

const MAX_NAME_LEN: usize = 16;
const MAX_ARGUMENTS: usize = 8;
const MAX_ARGUMENT_SIZE: usize = 16;
const MAX_PROCS: usize = 4; // fixed size for now
const REGISTER_COUNT: usize = 16;

type Program = *const u8;
type Pid = usize;

// TODO - abstract register system
#[repr(C)] // Just like C, but "safe"
#[derive(Copy, Clone)]
pub struct ARegs
{
        pub eax: u32,
        pub ebx: u32,
        pub ecx: u32,
        pub edx: u32,
        pub esp: u32,
        pub ebp: u32,
        pub esi: u32,
        pub edi: u32,
        pub eip: u32,
        pub cs:  u32,
        pub ds:  u32,
        pub es:  u32,
        pub ss:  u32,
        pub fs:  u32,
        pub gs:  u32,
        pub flags: u32,
}

#[repr(C)]
pub union Regs
{
        pub actual: ARegs,
        pub r: [u32; REGISTER_COUNT],
}

impl Regs
{
        pub fn new() -> Self
        {
                Self { r: [0; REGISTER_COUNT] }
        }
    
        pub fn get(&self, index: usize) -> u32
        {
                unsafe { self.r[index] } // SAFETY: Trust me bro
        }
    
        pub fn set(&mut self, index: usize, val: u32)
        {
                unsafe { self.r[index] = val }; // SAFETY: It's fine (probably)
        }
}

pub struct Stack
{
        pub size: u32,
        pub raw:  *mut u8,
}

impl Stack
{
        pub fn new() -> Self
        {
                Self
                {
                        size: 0,
                        raw: core::ptr::null_mut(), // The null pointer: C's gift to Rust
                }
        }
    
        pub fn push(&mut self, value: u8)
        {
                unsafe
                {
                        // TODO: Actually implement stack pushing
                        // This is the "doesn't matter" part after all
                        if !self.raw.is_null()
                        {
                            core::ptr::write(self.raw, value);
                        }
                }
        }
    
        pub fn pop(&mut self) -> Option<u8>
        {
                unsafe
                {
                        if self.raw.is_null()
                        {
                                None
                        }
                        else
                        {
                                Some(core::ptr::read(self.raw))
                                // TODO: Update stack pointer
                                // Maybe later
                        }
                }
        }
}

pub struct ProcessInformation
{
        pub active: bool,
        pub valid: bool,
        pub delete: bool,
        pub debug: bool,
        pub x: u32, pub y: u32, // i forget what these do
        pub pid: Pid,
        pub parent: Pid,
}

impl ProcessInformation
{
        pub fn new() -> Self
        {
                Self
                {
                        active: false,
                        valid: false,
                        delete: false,
                        debug: false,
                        x: 0, y: 0, // Still don't know what these do
                        pid: 0,
                        parent: 0,
                }
        }
    
        pub fn mark_for_death(&mut self)
        {
                self.delete = true;
        }
    
        pub fn is_zombie(&self) -> bool
        {
                self.delete && !self.active
        }
}

pub struct ProcessEnviroment
{
        pub arguments: [[u8; MAX_ARGUMENT_SIZE]; MAX_ARGUMENTS],
        pub count: u32,
}

impl ProcessEnviroment
{
        pub fn new() -> Self
        {
                Self
                {
                        arguments: [[0; MAX_ARGUMENT_SIZE]; MAX_ARGUMENTS],
                        count: 0,
                }
        }
    
        pub fn add_argument(&mut self, arg: &[u8])
        {
                if self.count < MAX_ARGUMENTS as u32
                {
                        let len = arg.len().min(MAX_ARGUMENT_SIZE - 1);
                        self.arguments[self.count as usize][..len].copy_from_slice(&arg[..len]);
                        self.arguments[self.count as usize][len] = 0; // FUCK YOU PEOPLE WHO DECIDED NULL TERMINATION WAS A GOOD IDEA
                        self.count += 1;
                }
        }
    
        pub fn get_argument(&self, index: usize) -> Option<&[u8]>
        {
                if index < self.count as usize
                {
                    // Find the null terminator like it's 1972
                    let arg = &self.arguments[index];
                    let len = arg.iter().position(|&b| b == 0).unwrap_or(MAX_ARGUMENT_SIZE);
                    Some(&arg[..len])
                }
                else
                {
                    None
                }
        }
}

pub struct ScheduledProcess
{
        pub name: [u8; MAX_NAME_LEN],
        pub stack: Stack,
        pub prog: Program,
        pub regs: Regs,
        pub info: ProcessInformation,
        pub env: ProcessEnviroment,
}

impl ScheduledProcess
{
        pub fn new() -> Self
        {
                Self
                {
                        name: [0; MAX_NAME_LEN],
                        stack: Stack::new(),
                        prog: core::ptr::null(), // The program at address 0: very demure
                        regs: Regs::new(),
                        info: ProcessInformation::new(),
                        env: ProcessEnviroment::new(),
                }
        }
        
        pub fn set_name(&mut self, name: &[u8]) {
                let len = name.len().min(MAX_NAME_LEN - 1);
                self.name[..len].copy_from_slice(&name[..len]);
                self.name[len] = 0; // Null terminate because C didn't die for this
        }
        
        pub fn get_name(&self) -> &[u8]
        {
                let len = self.name.iter()
                        .position(|&b| b == 0)
                        .unwrap_or(MAX_NAME_LEN);
                &self.name[..len]
        }
        
        pub fn is_runnable(&self) -> bool
        {
                self.info.active && self.info.valid && !self.info.delete
        }
}

pub struct Scheduler
{
        pub procs: [ScheduledProcess; MAX_PROCS],
        pub current_proc: Pid,
        pub tick_counter: u32,
}

impl Scheduler
{
        pub fn new() -> Self
        {
                // This is beautiful. This is wrong. This is Rust.
                Self
                {
                        procs: [
                            ScheduledProcess::new(),
                            ScheduledProcess::new(),
                            ScheduledProcess::new(),
                            ScheduledProcess::new(),
                        ],
                        current_proc: 0,
                        tick_counter: 0,
                }
        }
        
        pub fn find_free_slot(&self) -> Option<usize>
        {
                self.procs.iter().position(|p| !p.info.valid)
        }
        
        pub fn create_process(&mut self, name: &[u8]) -> Option<Pid>
        {
                if let Some(slot) = self.find_free_slot()
                {
                        self.procs[slot] = ScheduledProcess::new();
                        self.procs[slot].set_name(name);
                        self.procs[slot].info.valid = true;
                        self.procs[slot].info.pid = slot;
                        Some(slot)
                }
                else
                {
                        None
                }
        }
        
        pub fn kill_process(&mut self, pid: Pid)
        {
                if pid < MAX_PROCS
                {
                        self.procs[pid].info.mark_for_death();
                }
        }
        
        pub fn tick(&mut self)
        {
            self.tick_counter = self.tick_counter.wrapping_add(1);
        }
        
        pub fn next_process(&self) -> Pid
        {
            (self.current_proc + 1) % MAX_PROCS
        }
        
        pub fn get_current_process(&self) -> Option<&ScheduledProcess>
        {
                if self.current_proc < MAX_PROCS
                {
                        Some(&self.procs[self.current_proc])
                }
                else
                {
                        None // How did we get here?
                }
        }
        
        pub fn get_current_process_mut(&mut self) -> Option<&mut ScheduledProcess>
        {
                if self.current_proc < MAX_PROCS
                {
                        Some(&mut self.procs[self.current_proc])
                }
                else
                {
                        None
                }
        }
}

extern "C"
{
        fn malloc(size: usize) -> *mut c_void;
        fn free(p: *const c_void);
}