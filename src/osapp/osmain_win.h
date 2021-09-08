/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osmain_win.h
 * https://nappgui.com/en/osapp/osmain_win.html
 *
 */

/* Cross-platform main */

#include "nowarn.hxx"
#include <Windows.h>
#include "warn.hxx"

#define osmain(func_create, func_destroy, options, type)\
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)\
{\
    (void)(hPrevInstance),\
    (void)(lpCmdLine),\
    (void)(nCmdShow),\
    FUNC_CHECK_APP_CREATE(func_create, type),\
    FUNC_CHECK_DESTROY(func_destroy, type),\
	osmain_imp(\
                0, NULL, (void*)hInstance, 0.,\
                (FPtr_app_create)func_create,\
                (FPtr_app_update)NULL,\
                (FPtr_destroy)func_destroy,\
                options);\
    return 0;\
}

#define osmain_sync(lframe, func_create, func_destroy, func_update, options, type)\
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)\
{\
    (void)(hPrevInstance),\
    (void)(lpCmdLine),\
    (void)(nCmdShow),\
    FUNC_CHECK_APP_CREATE(func_create, type),\
    FUNC_CHECK_APP_UPDATE(func_update, type),\
    FUNC_CHECK_DESTROY(func_destroy, type),\
	osmain_imp(\
                0, NULL, (void*)hInstance, lframe,\
                (FPtr_app_create)func_create,\
                (FPtr_app_update)func_update,\
                (FPtr_destroy)func_destroy,\
                options);\
    return 0;\
}
