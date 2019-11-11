#include "indigo_utils.h"

#ifdef INDIGO_DEBUG
u8 __g_indigo_assert_failed = 0;

void __indigo_assert_fail(const char *__assertion, const char *__file,
                          unsigned int __line, const char *__function)
{
    printk(KERN_CRIT "%s:%u: %s: Assertion `%s' failed.\n", __file, __line, __function, __assertion);
    WRITE_ONCE(__g_indigo_assert_failed, 1);
}
#endif

void __indigo_trace_info(const char* format, ...)
{
    va_list args;
#ifdef INDIGO_DEBUG
    if (READ_ONCE(__g_indigo_assert_failed))
    {
        return;
    }
#endif
    va_start(args, format);
    vprintk(format, args);
    va_end(args);
}

// Aligns a not-16-byte-aligned rsp for function call, but always correctly restores the original rsp after the call.
// The trick is the following:
// (1) Push original rsp into stack twice. Now 0(%rsp) and 8(%rsp) all stores the original (%rsp).
// (2) Align rsp by doing a bitwise AND, do the function call with aligned rsp.
//     Since stack grows down, nothing is potentially overwritten by doing the AND to rsp.
// (3) After the bitwise AND, the rsp is unchanged if it was aligned, so 0(%rsp) and 8(%rsp)
//     still stores the original rsp; if rsp was unaligned, it is decremented by 8, so in this case
//     8(%rsp) and 16(%rsp) stores the original rsp .
// (4) So after the function call, doing %rsp = 8(%rsp) restores the original rsp correctly under both cases.
//
void function_call_5_params_respecting_stack_alignment(
        void* fn, u64 param1, u64 param2, u64 param3, u64 param4, u64 param5)
{
    asm volatile
    (
        // System V calling convention requires the stack input arguments
        // area (which we have none) ends at 16-byte boundary. Align rsp to 16 bytes.
        //
        "push %%rsp\n\t"
        "push (%%rsp)\n\t"
        "and $0xfffffffffffffff0ULL, %%rsp\n\t"

        // Call using System V calling convention
        //
        "mov %1, %%rdi\n\t"
        "mov %2, %%rsi\n\t"
        "mov %3, %%rdx\n\t"
        "mov %4, %%rcx\n\t"
        "mov %5, %%r8\n\t"
        "call *%0\n\t"

        // Restore rsp. This works no matter rsp is originally 16-byte aligned or not.
        //
        "mov 8(%%rsp), %%rsp"

        : /* no output */

        // Input list
        //
        : "r" (fn)
        , "mri" (param1)
        , "mri" (param2)
        , "mri" (param3)
        , "mri" (param4)
        , "mri" (param5)

        // Clobbered list, System V calling convention clobbers
        // all registers holding parameters as well as r10, r11
        //
        : "rdi"
        , "rsi"
        , "rdx"
        , "rcx"
        , "r8"
        , "r9"
        , "r10"
        , "r11"
    );
}
