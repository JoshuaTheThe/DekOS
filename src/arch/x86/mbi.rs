
#![no_std]

#[repr(C, packed)]
#[derive(Clone, Copy)]

pub struct MultibootInfo
{
        pub flags: u32,
        pub mem_lower: u32,
        pub mem_upper: u32,
        pub boot_device: u32,
        pub cmdline: u32,
        pub mods_count: u32,
        pub mods_addr: u32,
        pub num: u32,
        pub size: u32,
        pub addr: u32,
        pub shndx: u32,
        pub mmap_length: u32,
        pub mmap_addr: u32,
        pub drives_length: u32,
        pub drives_addr: u32,
        pub config_table: u32,
        pub boot_loader_name: u32,
        pub apm_table: u32,
        pub vbe_control_info: u32,
        pub vbe_mode_info: u32,
        pub vbe_mode: u16,
        pub vbe_interface_seg: u16,
        pub vbe_interface_off: u16,
        pub vbe_interface_len: u16,
        pub framebuffer_addr: u64,
        pub framebuffer_pitch: u32,
        pub framebuffer_width: u32,
        pub framebuffer_height: u32,
        pub framebuffer_bpp: u8,
        pub framebuffer_type: u8,
        pub color_info: ColorInfo,
}

#[repr(C, packed)]
#[derive(Clone, Copy)]
pub union ColorInfo
{
        pub rgb_info: RGBInfo,
        pub indexed_info: IndexedInfo,
}

#[repr(C, packed)]
#[derive(Clone, Copy)]
pub struct RGBInfo
{
        pub red_field_position: u8,
        pub red_mask_size: u8,
        pub green_field_position: u8,
        pub green_mask_size: u8,
        pub blue_field_position: u8,
        pub blue_mask_size: u8,
}

#[repr(C, packed)]
#[derive(Clone, Copy)]
pub struct IndexedInfo
{
        pub reserved: u8,
        pub memory_model: u8,
        pub reserved1: [u8; 3],
}

impl MultibootInfo
{
        pub fn framebuffer_addr(&self) -> u64
        {
                unsafe { self.framebuffer_addr }
        }
    
        pub fn framebuffer_width(&self) -> u32
        {
                unsafe { self.framebuffer_width }
        }
    
        pub fn framebuffer_height(&self) -> u32
        {
                unsafe { self.framebuffer_height }
        }
    
        pub fn framebuffer_bpp(&self) -> u8
        {
                unsafe { self.framebuffer_bpp }
        }
    
        pub fn mem_upper(&self) -> u32
        {
                unsafe { self.mem_upper }
        }
    
        pub fn mem_lower(&self) -> u32
        {
                unsafe { self.mem_lower }
        }
}
