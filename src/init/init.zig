
// Init.zig
// wont actually do anything, just a test

extern fn printf(format: [*:0]const u8, ...) void;

pub export fn zmain() void
{
        printf(" [INFO] Hello, From Zig!\n");
}
