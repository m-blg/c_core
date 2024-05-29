#pragma once

/// @example
/// #if CORE_IMPL_GUARD(CORE_ARRAY_SLICE)
/// #define CORE_ARRAY_SLICE_I
///
/// ...
/// stuff
/// ...
///
/// #endif // CORE_ARRAY_SLICE_IMPL
#define CORE_IMPL_GUARD(SECTION) \
    !defined(CORE_DECL_ONLY) && \
    defined(SECTION##_IMPL) && \
    !defined(SECTION##_I) \

#define CORE_IS_GUARD(SECTION, SUBSECTION) \
    !defined(CORE_DECL_ONLY) && \
    defined(SECTION##_##SUBSECTION##_IMPL) &&   \
    !defined(SECTION##_##SUBSECTION##_I) &&   \
    !defined(SECTION##_H)                   \


#define CORE_HEADER_GUARD(SECTION) \
    !defined(CORE_DECL_ONLY) && \
    !defined(SECTION##_H) \


/// @example
/// #if CORE_HS_GUARD(CORE_ARRAY, SLICE)
/// #define CORE_ARRAY_H
/// #define CORE_ARRAY_SLICE_H
///
/// ...
/// stuff
/// ...
///
/// #undef CORE_ARRAY_H
/// #endif // CORE_ARRAY_SLICE_H
#define CORE_HS_GUARD(SECTION, SUBSECTION) \
    !defined(CORE_DECL_ONLY) && \
    !defined(SECTION##_H) && \
    !defined(SECTION##_##SUBSECTION##_H)     



#ifdef CORE_IMPL
#undef CORE_IMPL

#include "core/core.h"
#define CORE_IMPL
#include "core/core.h"

#endif // CORE_IMPL