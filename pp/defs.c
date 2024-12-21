#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "panic.h"

static char **tokenify(const char *s);

/*
 * Add a #define
 *
 * Examples:
 * #define FOO                      =>  name="foo"   args=NULL       replace=NULL
 * #define FOO bar baz              =>  name="FOO"   args=NULL       replace="bar baz"
 * #define FOO(bar bux) baz bar bux =>  name="FOO"   args="bar bux"  replace="baz bar bux"
 */
void add_define(defines *defs, const char *name, const char *args, const char *replace)
{
    char *tok;
    int nargs = 0;

    // resize if needed
    if (defs->cur == defs->max) {
        defs->max *= 2;
        defs->defs = realloc(defs->defs, sizeof(def) * defs->max);
        if (!defs->defs) {
            perror("alloc failed");
            exit(2);
        }
    }
        
    def *d = &defs->defs[defs->cur++];
    d->name = strdup(name);

    d->args = tokenify(args);
    d->replace = tokenify(replace);
}

static char **tokenify(const char *s)
{
    char **result = NULL;
    char *tok;
    int ntok = 0;
    int len;

    if (!s) {
        return NULL;
    }

    while (*s) {
        if (*s == '"') {
            len = strcspn(s+1, "\"");
            if (*(s+1+len) != '"') {
                panic("missing end quote");
            }
            len += 2;
        } else {
            len = strcspn(s, " \t\v\r\n");
        }

        if (ntok % 5 == 0) {
            result = realloc(result, sizeof(char *) * (ntok + 5));
        }
        result[ntok] = malloc(len+1);
        strncpy(result[ntok], s, len);
        result[ntok][len] = '\0';
        ntok++;

        s += len;
        s += strspn(s, " \t\v\r\n");
    }

    return result;
}

defines *init_defines(void)
{
    defines *defs;

    defs = calloc(1, sizeof *defs);

    // initial defines from 'echo | clang -dM -E -'

    add_define(defs, "_LP64", NULL, "1");
    add_define(defs, "__AARCH64EL__", NULL, "1");
    add_define(defs, "__AARCH64_CMODEL_SMALL__", NULL, "1");
    add_define(defs, "__AARCH64_SIMD__", NULL, "1");
    add_define(defs, "__APPLE_CC__", NULL, "6000");
    add_define(defs, "__APPLE__", NULL, "1");
    add_define(defs, "__ARM64_ARCH_8__", NULL, "1");
    add_define(defs, "__ARM_64BIT_STATE", NULL, "1");
    add_define(defs, "__ARM_ACLE", NULL, "200");
    add_define(defs, "__ARM_ALIGN_MAX_STACK_PWR", NULL, "4");
    add_define(defs, "__ARM_ARCH", NULL, "8");
    add_define(defs, "__ARM_ARCH_8_3__", NULL, "1");
    add_define(defs, "__ARM_ARCH_8_4__", NULL, "1");
    add_define(defs, "__ARM_ARCH_8_5__", NULL, "1");
    add_define(defs, "__ARM_ARCH_ISA_A64", NULL, "1");
    add_define(defs, "__ARM_ARCH_PROFILE", NULL, "'A'");
    add_define(defs, "__ARM_FEATURE_AES", NULL, "1");
    add_define(defs, "__ARM_FEATURE_ATOMICS", NULL, "1");
    add_define(defs, "__ARM_FEATURE_BTI", NULL, "1");
    add_define(defs, "__ARM_FEATURE_CLZ", NULL, "1");
    add_define(defs, "__ARM_FEATURE_COMPLEX", NULL, "1");
    add_define(defs, "__ARM_FEATURE_CRC32", NULL, "1");
    add_define(defs, "__ARM_FEATURE_CRYPTO", NULL, "1");
    add_define(defs, "__ARM_FEATURE_DIRECTED_ROUNDING", NULL, "1");
    add_define(defs, "__ARM_FEATURE_DIV", NULL, "1");
    add_define(defs, "__ARM_FEATURE_DOTPROD", NULL, "1");
    add_define(defs, "__ARM_FEATURE_FMA", NULL, "1");
    add_define(defs, "__ARM_FEATURE_FP16_FML", NULL, "1");
    add_define(defs, "__ARM_FEATURE_FP16_SCALAR_ARITHMETIC", NULL, "1");
    add_define(defs, "__ARM_FEATURE_FP16_VECTOR_ARITHMETIC", NULL, "1");
    add_define(defs, "__ARM_FEATURE_FRINT", NULL, "1");
    add_define(defs, "__ARM_FEATURE_IDIV", NULL, "1");
    add_define(defs, "__ARM_FEATURE_JCVT", NULL, "1");
    add_define(defs, "__ARM_FEATURE_LDREX", NULL, "0xF");
    add_define(defs, "__ARM_FEATURE_NUMERIC_MAXMIN", NULL, "1");
    add_define(defs, "__ARM_FEATURE_PAUTH", NULL, "1");
    add_define(defs, "__ARM_FEATURE_QRDMX", NULL, "1");
    add_define(defs, "__ARM_FEATURE_RCPC", NULL, "1");
    add_define(defs, "__ARM_FEATURE_SHA2", NULL, "1");
    add_define(defs, "__ARM_FEATURE_SHA3", NULL, "1");
    add_define(defs, "__ARM_FEATURE_SHA512", NULL, "1");
    add_define(defs, "__ARM_FEATURE_UNALIGNED", NULL, "1");
    add_define(defs, "__ARM_FP", NULL, "0xE");
    add_define(defs, "__ARM_FP16_ARGS", NULL, "1");
    add_define(defs, "__ARM_FP16_FORMAT_IEEE", NULL, "1");
    add_define(defs, "__ARM_NEON", NULL, "1");
    add_define(defs, "__ARM_NEON_FP", NULL, "0xE");
    add_define(defs, "__ARM_NEON__", NULL, "1");
    add_define(defs, "__ARM_PCS_AAPCS64", NULL, "1");
    add_define(defs, "__ARM_SIZEOF_MINIMAL_ENUM", NULL, "4");
    add_define(defs, "__ARM_SIZEOF_WCHAR_T", NULL, "4");
    add_define(defs, "__ATOMIC_ACQUIRE", NULL, "2");
    add_define(defs, "__ATOMIC_ACQ_REL", NULL, "4");
    add_define(defs, "__ATOMIC_CONSUME", NULL, "1");
    add_define(defs, "__ATOMIC_RELAXED", NULL, "0");
    add_define(defs, "__ATOMIC_RELEASE", NULL, "3");
    add_define(defs, "__ATOMIC_SEQ_CST", NULL, "5");
    add_define(defs, "__BIGGEST_ALIGNMENT__", NULL, "8");
    add_define(defs, "__BITINT_MAXWIDTH__", NULL, "128");
    add_define(defs, "__BLOCKS__", NULL, "1");
    add_define(defs, "__BOOL_WIDTH__", NULL, "8");
    add_define(defs, "__BYTE_ORDER__", NULL, "__ORDER_LITTLE_ENDIAN__");
    add_define(defs, "__CHAR16_TYPE__", NULL, "unsigned short");
    add_define(defs, "__CHAR32_TYPE__", NULL, "unsigned int");
    add_define(defs, "__CHAR_BIT__", NULL, "8");
    add_define(defs, "__CONSTANT_CFSTRINGS__", NULL, "1");
    add_define(defs, "__DBL_DECIMAL_DIG__", NULL, "17");
    add_define(defs, "__DBL_DENORM_MIN__", NULL, "4.9406564584124654e-324");
    add_define(defs, "__DBL_DIG__", NULL, "15");
    add_define(defs, "__DBL_EPSILON__", NULL, "2.2204460492503131e-16");
    add_define(defs, "__DBL_HAS_DENORM__", NULL, "1");
    add_define(defs, "__DBL_HAS_INFINITY__", NULL, "1");
    add_define(defs, "__DBL_HAS_QUIET_NAN__", NULL, "1");
    add_define(defs, "__DBL_MANT_DIG__", NULL, "53");
    add_define(defs, "__DBL_MAX_10_EXP__", NULL, "308");
    add_define(defs, "__DBL_MAX_EXP__", NULL, "1024");
    add_define(defs, "__DBL_MAX__", NULL, "1.7976931348623157e+308");
    add_define(defs, "__DBL_MIN_10_EXP__", NULL, "(-307)");
    add_define(defs, "__DBL_MIN_EXP__", NULL, "(-1021)");
    add_define(defs, "__DBL_MIN__", NULL, "2.2250738585072014e-308");
    add_define(defs, "__DECIMAL_DIG__", NULL, "__LDBL_DECIMAL_DIG__");
    add_define(defs, "__DYNAMIC__", NULL, "1");
    add_define(defs, "__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__", NULL, "150000");
    add_define(defs, "__ENVIRONMENT_OS_VERSION_MIN_REQUIRED__", NULL, "150000");
    add_define(defs, "__FINITE_MATH_ONLY__", NULL, "0");
    add_define(defs, "__FLT16_DECIMAL_DIG__", NULL, "5");
    add_define(defs, "__FLT16_DENORM_MIN__", NULL, "5.9604644775390625e-8F16");
    add_define(defs, "__FLT16_DIG__", NULL, "3");
    add_define(defs, "__FLT16_EPSILON__", NULL, "9.765625e-4F16");
    add_define(defs, "__FLT16_HAS_DENORM__", NULL, "1");
    add_define(defs, "__FLT16_HAS_INFINITY__", NULL, "1");
    add_define(defs, "__FLT16_HAS_QUIET_NAN__", NULL, "1");
    add_define(defs, "__FLT16_MANT_DIG__", NULL, "11");
    add_define(defs, "__FLT16_MAX_10_EXP__", NULL, "4");
    add_define(defs, "__FLT16_MAX_EXP__", NULL, "16");
    add_define(defs, "__FLT16_MAX__", NULL, "6.5504e+4F16");
    add_define(defs, "__FLT16_MIN_10_EXP__", NULL, "(-4)");
    add_define(defs, "__FLT16_MIN_EXP__", NULL, "(-13)");
    add_define(defs, "__FLT16_MIN__", NULL, "6.103515625e-5F16");
    add_define(defs, "__FLT_DECIMAL_DIG__", NULL, "9");
    add_define(defs, "__FLT_DENORM_MIN__", NULL, "1.40129846e-45F");
    add_define(defs, "__FLT_DIG__", NULL, "6");
    add_define(defs, "__FLT_EPSILON__", NULL, "1.19209290e-7F");
    add_define(defs, "__FLT_HAS_DENORM__", NULL, "1");
    add_define(defs, "__FLT_HAS_INFINITY__", NULL, "1");
    add_define(defs, "__FLT_HAS_QUIET_NAN__", NULL, "1");
    add_define(defs, "__FLT_MANT_DIG__", NULL, "24");
    add_define(defs, "__FLT_MAX_10_EXP__", NULL, "38");
    add_define(defs, "__FLT_MAX_EXP__", NULL, "128");
    add_define(defs, "__FLT_MAX__", NULL, "3.40282347e+38F");
    add_define(defs, "__FLT_MIN_10_EXP__", NULL, "(-37)");
    add_define(defs, "__FLT_MIN_EXP__", NULL, "(-125)");
    add_define(defs, "__FLT_MIN__", NULL, "1.17549435e-38F");
    add_define(defs, "__FLT_RADIX__", NULL, "2");
    add_define(defs, "__FPCLASS_NEGINF", NULL, "0x0004");
    add_define(defs, "__FPCLASS_NEGNORMAL", NULL, "0x0008");
    add_define(defs, "__FPCLASS_NEGSUBNORMAL", NULL, "0x0010");
    add_define(defs, "__FPCLASS_NEGZERO", NULL, "0x0020");
    add_define(defs, "__FPCLASS_POSINF", NULL, "0x0200");
    add_define(defs, "__FPCLASS_POSNORMAL", NULL, "0x0100");
    add_define(defs, "__FPCLASS_POSSUBNORMAL", NULL, "0x0080");
    add_define(defs, "__FPCLASS_POSZERO", NULL, "0x0040");
    add_define(defs, "__FPCLASS_QNAN", NULL, "0x0002");
    add_define(defs, "__FPCLASS_SNAN", NULL, "0x0001");
    add_define(defs, "__FP_FAST_FMA", NULL, "1");
    add_define(defs, "__FP_FAST_FMAF", NULL, "1");
    add_define(defs, "__GCC_ASM_FLAG_OUTPUTS__", NULL, "1");
    add_define(defs, "__GCC_ATOMIC_BOOL_LOCK_FREE", NULL, "2");
    add_define(defs, "__GCC_ATOMIC_CHAR16_T_LOCK_FREE", NULL, "2");
    add_define(defs, "__GCC_ATOMIC_CHAR32_T_LOCK_FREE", NULL, "2");
    add_define(defs, "__GCC_ATOMIC_CHAR_LOCK_FREE", NULL, "2");
    add_define(defs, "__GCC_ATOMIC_INT_LOCK_FREE", NULL, "2");
    add_define(defs, "__GCC_ATOMIC_LLONG_LOCK_FREE", NULL, "2");
    add_define(defs, "__GCC_ATOMIC_LONG_LOCK_FREE", NULL, "2");
    add_define(defs, "__GCC_ATOMIC_POINTER_LOCK_FREE", NULL, "2");
    add_define(defs, "__GCC_ATOMIC_SHORT_LOCK_FREE", NULL, "2");
    add_define(defs, "__GCC_ATOMIC_TEST_AND_SET_TRUEVAL", NULL, "1");
    add_define(defs, "__GCC_ATOMIC_WCHAR_T_LOCK_FREE", NULL, "2");
    add_define(defs, "__GCC_HAVE_DWARF2_CFI_ASM", NULL, "1");
    add_define(defs, "__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1", NULL, "1");
    add_define(defs, "__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2", NULL, "1");
    add_define(defs, "__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4", NULL, "1");
    add_define(defs, "__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8", NULL, "1");
    add_define(defs, "__GNUC_MINOR__", NULL, "2");
    add_define(defs, "__GNUC_PATCHLEVEL__", NULL, "1");
    add_define(defs, "__GNUC_STDC_INLINE__", NULL, "1");
    add_define(defs, "__GNUC__", NULL, "4");
    add_define(defs, "__GXX_ABI_VERSION", NULL, "1002");
    add_define(defs, "__HAVE_FUNCTION_MULTI_VERSIONING", NULL, "1");
    add_define(defs, "__INT16_C_SUFFIX__", NULL, "");
    add_define(defs, "__INT16_FMTd__", NULL, "\"hd\"");
    add_define(defs, "__INT16_FMTi__", NULL, "\"hi\"");
    add_define(defs, "__INT16_MAX__", NULL, "32767");
    add_define(defs, "__INT16_TYPE__", NULL, "short");
    add_define(defs, "__INT32_C_SUFFIX__", NULL, "");
    add_define(defs, "__INT32_FMTd__", NULL, "\"d\"");
    add_define(defs, "__INT32_FMTi__", NULL, "\"i\"");
    add_define(defs, "__INT32_MAX__", NULL, "2147483647");
    add_define(defs, "__INT32_TYPE__", NULL, "int");
    add_define(defs, "__INT64_C_SUFFIX__", NULL, "LL");
    add_define(defs, "__INT64_FMTd__", NULL, "\"lld\"");
    add_define(defs, "__INT64_FMTi__", NULL, "\"lli\"");
    add_define(defs, "__INT64_MAX__", NULL, "9223372036854775807LL");
    add_define(defs, "__INT64_TYPE__", NULL, "long long int");
    add_define(defs, "__INT8_C_SUFFIX__", NULL, "");
    add_define(defs, "__INT8_FMTd__", NULL, "\"hhd\"");
    add_define(defs, "__INT8_FMTi__", NULL, "\"hhi\"");
    add_define(defs, "__INT8_MAX__", NULL, "127");
    add_define(defs, "__INT8_TYPE__", NULL, "signed char");
    add_define(defs, "__INTMAX_C_SUFFIX__", NULL, "L");
    add_define(defs, "__INTMAX_FMTd__", NULL, "\"ld\"");
    add_define(defs, "__INTMAX_FMTi__", NULL, "\"li\"");
    add_define(defs, "__INTMAX_MAX__", NULL, "9223372036854775807L");
    add_define(defs, "__INTMAX_TYPE__", NULL, "long int");
    add_define(defs, "__INTMAX_WIDTH__", NULL, "64");
    add_define(defs, "__INTPTR_FMTd__", NULL, "\"ld\"");
    add_define(defs, "__INTPTR_FMTi__", NULL, "\"li\"");
    add_define(defs, "__INTPTR_MAX__", NULL, "9223372036854775807L");
    add_define(defs, "__INTPTR_TYPE__", NULL, "long int");
    add_define(defs, "__INTPTR_WIDTH__", NULL, "64");
    add_define(defs, "__INT_FAST16_FMTd__", NULL, "\"hd\"");
    add_define(defs, "__INT_FAST16_FMTi__", NULL, "\"hi\"");
    add_define(defs, "__INT_FAST16_MAX__", NULL, "32767");
    add_define(defs, "__INT_FAST16_TYPE__", NULL, "short");
    add_define(defs, "__INT_FAST16_WIDTH__", NULL, "16");
    add_define(defs, "__INT_FAST32_FMTd__", NULL, "\"d\"");
    add_define(defs, "__INT_FAST32_FMTi__", NULL, "\"i\"");
    add_define(defs, "__INT_FAST32_MAX__", NULL, "2147483647");
    add_define(defs, "__INT_FAST32_TYPE__", NULL, "int");
    add_define(defs, "__INT_FAST32_WIDTH__", NULL, "32");
    add_define(defs, "__INT_FAST64_FMTd__", NULL, "\"lld\"");
    add_define(defs, "__INT_FAST64_FMTi__", NULL, "\"lli\"");
    add_define(defs, "__INT_FAST64_MAX__", NULL, "9223372036854775807LL");
    add_define(defs, "__INT_FAST64_TYPE__", NULL, "long long int");
    add_define(defs, "__INT_FAST64_WIDTH__", NULL, "64");
    add_define(defs, "__INT_FAST8_FMTd__", NULL, "\"hhd\"");
    add_define(defs, "__INT_FAST8_FMTi__", NULL, "\"hhi\"");
    add_define(defs, "__INT_FAST8_MAX__", NULL, "127");
    add_define(defs, "__INT_FAST8_TYPE__", NULL, "signed char");
    add_define(defs, "__INT_FAST8_WIDTH__", NULL, "8");
    add_define(defs, "__INT_LEAST16_FMTd__", NULL, "\"hd\"");
    add_define(defs, "__INT_LEAST16_FMTi__", NULL, "\"hi\"");
    add_define(defs, "__INT_LEAST16_MAX__", NULL, "32767");
    add_define(defs, "__INT_LEAST16_TYPE__", NULL, "short");
    add_define(defs, "__INT_LEAST16_WIDTH__", NULL, "16");
    add_define(defs, "__INT_LEAST32_FMTd__", NULL, "\"d\"");
    add_define(defs, "__INT_LEAST32_FMTi__", NULL, "\"i\"");
    add_define(defs, "__INT_LEAST32_MAX__", NULL, "2147483647");
    add_define(defs, "__INT_LEAST32_TYPE__", NULL, "int");
    add_define(defs, "__INT_LEAST32_WIDTH__", NULL, "32");
    add_define(defs, "__INT_LEAST64_FMTd__", NULL, "\"lld\"");
    add_define(defs, "__INT_LEAST64_FMTi__", NULL, "\"lli\"");
    add_define(defs, "__INT_LEAST64_MAX__", NULL, "9223372036854775807LL");
    add_define(defs, "__INT_LEAST64_TYPE__", NULL, "long long int");
    add_define(defs, "__INT_LEAST64_WIDTH__", NULL, "64");
    add_define(defs, "__INT_LEAST8_FMTd__", NULL, "\"hhd\"");
    add_define(defs, "__INT_LEAST8_FMTi__", NULL, "\"hhi\"");
    add_define(defs, "__INT_LEAST8_MAX__", NULL, "127");
    add_define(defs, "__INT_LEAST8_TYPE__", NULL, "signed char");
    add_define(defs, "__INT_LEAST8_WIDTH__", NULL, "8");
    add_define(defs, "__INT_MAX__", NULL, "2147483647");
    add_define(defs, "__INT_WIDTH__", NULL, "32");
    add_define(defs, "__LDBL_DECIMAL_DIG__", NULL, "17");
    add_define(defs, "__LDBL_DENORM_MIN__", NULL, "4.9406564584124654e-324L");
    add_define(defs, "__LDBL_DIG__", NULL, "15");
    add_define(defs, "__LDBL_EPSILON__", NULL, "2.2204460492503131e-16L");
    add_define(defs, "__LDBL_HAS_DENORM__", NULL, "1");
    add_define(defs, "__LDBL_HAS_INFINITY__", NULL, "1");
    add_define(defs, "__LDBL_HAS_QUIET_NAN__", NULL, "1");
    add_define(defs, "__LDBL_MANT_DIG__", NULL, "53");
    add_define(defs, "__LDBL_MAX_10_EXP__", NULL, "308");
    add_define(defs, "__LDBL_MAX_EXP__", NULL, "1024");
    add_define(defs, "__LDBL_MAX__", NULL, "1.7976931348623157e+308L");
    add_define(defs, "__LDBL_MIN_10_EXP__", NULL, "(-307)");
    add_define(defs, "__LDBL_MIN_EXP__", NULL, "(-1021)");
    add_define(defs, "__LDBL_MIN__", NULL, "2.2250738585072014e-308L");
    add_define(defs, "__LITTLE_ENDIAN__", NULL, "1");
    add_define(defs, "__LLONG_WIDTH__", NULL, "64");
    add_define(defs, "__LONG_LONG_MAX__", NULL, "9223372036854775807LL");
    add_define(defs, "__LONG_MAX__", NULL, "9223372036854775807L");
    add_define(defs, "__LONG_WIDTH__", NULL, "64");
    add_define(defs, "__LP64__", NULL, "1");
    add_define(defs, "__MACH__", NULL, "1");
    add_define(defs, "__NO_INLINE__", NULL, "1");
    add_define(defs, "__NO_MATH_ERRNO__", NULL, "1");
    add_define(defs, "__OBJC_BOOL_IS_BOOL", NULL, "1");
    add_define(defs, "__OPENCL_MEMORY_SCOPE_ALL_SVM_DEVICES", NULL, "3");
    add_define(defs, "__OPENCL_MEMORY_SCOPE_DEVICE", NULL, "2");
    add_define(defs, "__OPENCL_MEMORY_SCOPE_SUB_GROUP", NULL, "4");
    add_define(defs, "__OPENCL_MEMORY_SCOPE_WORK_GROUP", NULL, "1");
    add_define(defs, "__OPENCL_MEMORY_SCOPE_WORK_ITEM", NULL, "0");
    add_define(defs, "__ORDER_BIG_ENDIAN__", NULL, "4321");
    add_define(defs, "__ORDER_LITTLE_ENDIAN__", NULL, "1234");
    add_define(defs, "__ORDER_PDP_ENDIAN__", NULL, "3412");
    add_define(defs, "__PIC__", NULL, "2");
    add_define(defs, "__POINTER_WIDTH__", NULL, "64");
    add_define(defs, "__PRAGMA_REDEFINE_EXTNAME", NULL, "1");
    add_define(defs, "__PTRDIFF_FMTd__", NULL, "\"ld\"");
    add_define(defs, "__PTRDIFF_FMTi__", NULL, "\"li\"");
    add_define(defs, "__PTRDIFF_MAX__", NULL, "9223372036854775807L");
    add_define(defs, "__PTRDIFF_TYPE__", NULL, "long int");
    add_define(defs, "__PTRDIFF_WIDTH__", NULL, "64");
    add_define(defs, "__REGISTER_PREFIX__", NULL, "");
    add_define(defs, "__SCHAR_MAX__", NULL, "127");
    add_define(defs, "__SHRT_MAX__", NULL, "32767");
    add_define(defs, "__SHRT_WIDTH__", NULL, "16");
    add_define(defs, "__SIG_ATOMIC_MAX__", NULL, "2147483647");
    add_define(defs, "__SIG_ATOMIC_WIDTH__", NULL, "32");
    add_define(defs, "__SIZEOF_DOUBLE__", NULL, "8");
    add_define(defs, "__SIZEOF_FLOAT__", NULL, "4");
    add_define(defs, "__SIZEOF_INT128__", NULL, "16");
    add_define(defs, "__SIZEOF_INT__", NULL, "4");
    add_define(defs, "__SIZEOF_LONG_DOUBLE__", NULL, "8");
    add_define(defs, "__SIZEOF_LONG_LONG__", NULL, "8");
    add_define(defs, "__SIZEOF_LONG__", NULL, "8");
    add_define(defs, "__SIZEOF_POINTER__", NULL, "8");
    add_define(defs, "__SIZEOF_PTRDIFF_T__", NULL, "8");
    add_define(defs, "__SIZEOF_SHORT__", NULL, "2");
    add_define(defs, "__SIZEOF_SIZE_T__", NULL, "8");
    add_define(defs, "__SIZEOF_WCHAR_T__", NULL, "4");
    add_define(defs, "__SIZEOF_WINT_T__", NULL, "4");
    add_define(defs, "__SIZE_FMTX__", NULL, "\"lX\"");
    add_define(defs, "__SIZE_FMTo__", NULL, "\"lo\"");
    add_define(defs, "__SIZE_FMTu__", NULL, "\"lu\"");
    add_define(defs, "__SIZE_FMTx__", NULL, "\"lx\"");
    add_define(defs, "__SIZE_MAX__", NULL, "18446744073709551615UL");
    add_define(defs, "__SIZE_TYPE__", NULL, "long unsigned int");
    add_define(defs, "__SIZE_WIDTH__", NULL, "64");
    add_define(defs, "__SSP__", NULL, "1");
    add_define(defs, "__STDC_HOSTED__", NULL, "1");
    add_define(defs, "__STDC_NO_THREADS__", NULL, "1");
    add_define(defs, "__STDC_UTF_16__", NULL, "1");
    add_define(defs, "__STDC_UTF_32__", NULL, "1");
    add_define(defs, "__STDC_VERSION__", NULL, "201710L");
    add_define(defs, "__STDC__", NULL, "1");
    add_define(defs, "__UINT16_C_SUFFIX__", NULL, "");
    add_define(defs, "__UINT16_FMTX__", NULL, "\"hX\"");
    add_define(defs, "__UINT16_FMTo__", NULL, "\"ho\"");
    add_define(defs, "__UINT16_FMTu__", NULL, "\"hu\"");
    add_define(defs, "__UINT16_FMTx__", NULL, "\"hx\"");
    add_define(defs, "__UINT16_MAX__", NULL, "65535");
    add_define(defs, "__UINT16_TYPE__", NULL, "unsigned short");
    add_define(defs, "__UINT32_C_SUFFIX__", NULL, "U");
    add_define(defs, "__UINT32_FMTX__", NULL, "\"X\"");
    add_define(defs, "__UINT32_FMTo__", NULL, "\"o\"");
    add_define(defs, "__UINT32_FMTu__", NULL, "\"u\"");
    add_define(defs, "__UINT32_FMTx__", NULL, "\"x\"");
    add_define(defs, "__UINT32_MAX__", NULL, "4294967295U");
    add_define(defs, "__UINT32_TYPE__", NULL, "unsigned int");
    add_define(defs, "__UINT64_C_SUFFIX__", NULL, "ULL");
    add_define(defs, "__UINT64_FMTX__", NULL, "\"llX\"");
    add_define(defs, "__UINT64_FMTo__", NULL, "\"llo\"");
    add_define(defs, "__UINT64_FMTu__", NULL, "\"llu\"");
    add_define(defs, "__UINT64_FMTx__", NULL, "\"llx\"");
    add_define(defs, "__UINT64_MAX__", NULL, "18446744073709551615ULL");
    add_define(defs, "__UINT64_TYPE__", NULL, "long long unsigned int");
    add_define(defs, "__UINT8_C_SUFFIX__", NULL, "");
    add_define(defs, "__UINT8_FMTX__", NULL, "\"hhX\"");
    add_define(defs, "__UINT8_FMTo__", NULL, "\"hho\"");
    add_define(defs, "__UINT8_FMTu__", NULL, "\"hhu\"");
    add_define(defs, "__UINT8_FMTx__", NULL, "\"hhx\"");
    add_define(defs, "__UINT8_MAX__", NULL, "255");
    add_define(defs, "__UINT8_TYPE__", NULL, "unsigned char");
    add_define(defs, "__UINTMAX_C_SUFFIX__", NULL, "UL");
    add_define(defs, "__UINTMAX_FMTX__", NULL, "\"lX\"");
    add_define(defs, "__UINTMAX_FMTo__", NULL, "\"lo\"");
    add_define(defs, "__UINTMAX_FMTu__", NULL, "\"lu\"");
    add_define(defs, "__UINTMAX_FMTx__", NULL, "\"lx\"");
    add_define(defs, "__UINTMAX_MAX__", NULL, "18446744073709551615UL");
    add_define(defs, "__UINTMAX_TYPE__", NULL, "long unsigned int");
    add_define(defs, "__UINTMAX_WIDTH__", NULL, "64");
    add_define(defs, "__UINTPTR_FMTX__", NULL, "\"lX\"");
    add_define(defs, "__UINTPTR_FMTo__", NULL, "\"lo\"");
    add_define(defs, "__UINTPTR_FMTu__", NULL, "\"lu\"");
    add_define(defs, "__UINTPTR_FMTx__", NULL, "\"lx\"");
    add_define(defs, "__UINTPTR_MAX__", NULL, "18446744073709551615UL");
    add_define(defs, "__UINTPTR_TYPE__", NULL, "long unsigned int");
    add_define(defs, "__UINTPTR_WIDTH__", NULL, "64");
    add_define(defs, "__UINT_FAST16_FMTX__", NULL, "\"hX\"");
    add_define(defs, "__UINT_FAST16_FMTo__", NULL, "\"ho\"");
    add_define(defs, "__UINT_FAST16_FMTu__", NULL, "\"hu\"");
    add_define(defs, "__UINT_FAST16_FMTx__", NULL, "\"hx\"");
    add_define(defs, "__UINT_FAST16_MAX__", NULL, "65535");
    add_define(defs, "__UINT_FAST16_TYPE__", NULL, "unsigned short");
    add_define(defs, "__UINT_FAST32_FMTX__", NULL, "\"X\"");
    add_define(defs, "__UINT_FAST32_FMTo__", NULL, "\"o\"");
    add_define(defs, "__UINT_FAST32_FMTu__", NULL, "\"u\"");
    add_define(defs, "__UINT_FAST32_FMTx__", NULL, "\"x\"");
    add_define(defs, "__UINT_FAST32_MAX__", NULL, "4294967295U");
    add_define(defs, "__UINT_FAST32_TYPE__", NULL, "unsigned int");
    add_define(defs, "__UINT_FAST64_FMTX__", NULL, "\"llX\"");
    add_define(defs, "__UINT_FAST64_FMTo__", NULL, "\"llo\"");
    add_define(defs, "__UINT_FAST64_FMTu__", NULL, "\"llu\"");
    add_define(defs, "__UINT_FAST64_FMTx__", NULL, "\"llx\"");
    add_define(defs, "__UINT_FAST64_MAX__", NULL, "18446744073709551615ULL");
    add_define(defs, "__UINT_FAST64_TYPE__", NULL, "long long unsigned int");
    add_define(defs, "__UINT_FAST8_FMTX__", NULL, "\"hhX\"");
    add_define(defs, "__UINT_FAST8_FMTo__", NULL, "\"hho\"");
    add_define(defs, "__UINT_FAST8_FMTu__", NULL, "\"hhu\"");
    add_define(defs, "__UINT_FAST8_FMTx__", NULL, "\"hhx\"");
    add_define(defs, "__UINT_FAST8_MAX__", NULL, "255");
    add_define(defs, "__UINT_FAST8_TYPE__", NULL, "unsigned char");
    add_define(defs, "__UINT_LEAST16_FMTX__", NULL, "\"hX\"");
    add_define(defs, "__UINT_LEAST16_FMTo__", NULL, "\"ho\"");
    add_define(defs, "__UINT_LEAST16_FMTu__", NULL, "\"hu\"");
    add_define(defs, "__UINT_LEAST16_FMTx__", NULL, "\"hx\"");
    add_define(defs, "__UINT_LEAST16_MAX__", NULL, "65535");
    add_define(defs, "__UINT_LEAST16_TYPE__", NULL, "unsigned short");
    add_define(defs, "__UINT_LEAST32_FMTX__", NULL, "\"X\"");
    add_define(defs, "__UINT_LEAST32_FMTo__", NULL, "\"o\"");
    add_define(defs, "__UINT_LEAST32_FMTu__", NULL, "\"u\"");
    add_define(defs, "__UINT_LEAST32_FMTx__", NULL, "\"x\"");
    add_define(defs, "__UINT_LEAST32_MAX__", NULL, "4294967295U");
    add_define(defs, "__UINT_LEAST32_TYPE__", NULL, "unsigned int");
    add_define(defs, "__UINT_LEAST64_FMTX__", NULL, "\"llX\"");
    add_define(defs, "__UINT_LEAST64_FMTo__", NULL, "\"llo\"");
    add_define(defs, "__UINT_LEAST64_FMTu__", NULL, "\"llu\"");
    add_define(defs, "__UINT_LEAST64_FMTx__", NULL, "\"llx\"");
    add_define(defs, "__UINT_LEAST64_MAX__", NULL, "18446744073709551615ULL");
    add_define(defs, "__UINT_LEAST64_TYPE__", NULL, "long long unsigned int");
    add_define(defs, "__UINT_LEAST8_FMTX__", NULL, "\"hhX\"");
    add_define(defs, "__UINT_LEAST8_FMTo__", NULL, "\"hho\"");
    add_define(defs, "__UINT_LEAST8_FMTu__", NULL, "\"hhu\"");
    add_define(defs, "__UINT_LEAST8_FMTx__", NULL, "\"hhx\"");
    add_define(defs, "__UINT_LEAST8_MAX__", NULL, "255");
    add_define(defs, "__UINT_LEAST8_TYPE__", NULL, "unsigned char");
    add_define(defs, "__USER_LABEL_PREFIX__", NULL, "_");
    add_define(defs, "__VERSION__", NULL, "ghostcc 0.1");
    add_define(defs, "__WCHAR_MAX__", NULL, "2147483647");
    add_define(defs, "__WCHAR_TYPE__", NULL, "int");
    add_define(defs, "__WCHAR_WIDTH__", NULL, "32");
    add_define(defs, "__WINT_MAX__", NULL, "2147483647");
    add_define(defs, "__WINT_TYPE__", NULL, "int");
    add_define(defs, "__WINT_WIDTH__", NULL, "32");
    add_define(defs, "__aarch64__", NULL, "1");
    add_define(defs, "__apple_build_version__", NULL, "16000026");
    add_define(defs, "__arm64", NULL, "1");
    add_define(defs, "__arm64__", NULL, "1");
    add_define(defs, "__block", NULL, NULL);
    add_define(defs, "__llvm__", NULL, "1");
    add_define(defs, "__nonnull", NULL, "_Nonnull");
    add_define(defs, "__null_unspecified", NULL, "_Null_unspecified");
    add_define(defs, "__nullable", NULL, "_Nullable");
    add_define(defs, "__pic__", NULL, "2");
    add_define(defs, "__strong", NULL, "");
    add_define(defs, "__unsafe_unretained", NULL, "");
    add_define(defs, "__weak", NULL, NULL);

    return defs;
}
