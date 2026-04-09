SECTIONS
{
    .static_modules_backend ALIGN(8) :
    {
        __start_static_modules_backend = .;
        KEEP(*(.static_modules_backend))
        __stop_static_modules_backend = .;
    }
} INSERT AFTER .rodata;