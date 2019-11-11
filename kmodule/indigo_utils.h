#pragma once

#include <linux/printk.h>
#include <linux/compiler.h>
#include <linux/vmalloc.h>
#include <asm-generic/int-ll64.h>

// Uncomment to enable debug information and debug asserts
//
#define INDIGO_DEBUG

// Uncomment to enable verbose debug information
//
//#define INDIGO_DEBUG_VERBOSE

#ifdef INDIGO_DEBUG
// Upon failing an assert, we trace an alert, and stop all future tracing to
// make sure the alert is visible at the tail of dmesg
//
extern u8 __g_indigo_assert_failed;
#define Assert(expr)					\
    ((expr)								\
     ? (void) (0)						\
     : __indigo_assert_fail (#expr, __FILE__, __LINE__, __extension__ __PRETTY_FUNCTION__))

void __indigo_assert_fail(const char *__assertion, const char *__file,
                          unsigned int __line, const char *__function);
#else
#define Assert(expr) ((void) (0))
#endif

void __indigo_trace_info(const char* format, ...);

#define TRACE_INFO(format, ...) __indigo_trace_info(format "\n", ##__VA_ARGS__)

#ifdef INDIGO_DEBUG
#   define TRACE_DEBUG(...) TRACE_INFO(__VA_ARGS__)
#   ifdef INDIGO_DEBUG_VERBOSE
#       define TRACE_DEBUG_VERBOSE(...) TRACE_INFO(__VA_ARGS__)
#   else
#       define TRACE_DEBUG_VERBOSE(...)
#   endif
#else
#   define TRACE_DEBUG(...)
#   define TRACE_DEBUG_VERBOSE(...)
#endif

#define WARN_UNUSED __attribute__((warn_unused_result))
#define ALWAYS_INLINE __attribute__((always_inline))

// GCC assumes that the stack is 16-byte aligned all the times.
// HOWEVER, this is not always true in kernel... probably due to some asm code not respecting this rule.
// Normally this is OK because kernel is seldom using vectorized instructions,
// but this breaks things for our Tensorflow AOT compiled functions: the generated
// assembly uses aligned SSE instructions on rsp, which breaks down when rsp is not
// 16-byte aligned. This function is a hack to fix this issue.
//
// Speficially, this function allows safely calling a function pointer using System V calling convention,
// without prior assumption on whether rsp is 16-byte aligned (but it has to be 8-byte aligned of course).
// The function pointer is supposed to take 5 64-bit parameters and has no return value.
//
// See comments in the .c file for implementation details.
//
void function_call_5_params_respecting_stack_alignment(
        void* fn, u64 param1, u64 param2, u64 param3, u64 param4, u64 param5);
