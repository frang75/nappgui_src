/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: warn.hxx
 *
 */

/* This header will ignore ALL warnings of code file (.c .cpp .m etc) */
/* USE ONLY IN THIRD PARTY FILES, NOT IN OWN FILES */

#if defined (_MSC_VER)
#pragma pop_macro("Set")
#pragma warning(pop) 
#pragma warning( disable : 4514 )
#pragma warning( default : 4625 )
#pragma warning( default : 4626 )
#pragma warning( default : 4668 )
#pragma warning( default : 4255 )

#if _MSC_VER > 1400
#pragma warning( default : 4458 )
#endif

#pragma warning( default : 4201 )
#pragma warning( default : 4548 )
#undef _CRT_SECURE_NO_WARNINGS
#endif

#if defined (__GNUC__)
#if defined(__APPLE__)
#if (__GNUC__ < 4) || (__GNUC__ == 4)

#if (__GNUC__ == 4) && (__GNUC_MINOR__ > 2)
#pragma GCC diagnostic pop
#endif

#pragma GCC diagnostic ignored "-Wcomment"
#endif
#endif
#endif

#if defined (__clang__)
#pragma GCC diagnostic pop
#endif

