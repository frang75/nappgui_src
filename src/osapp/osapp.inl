/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osapp.inl
 *
 */

/* Application runloop */

#include "osapp.ixx"

__EXTERN_C

OSApp *_osapp_init_imp(
    uint32_t argc,
    char_t **argv,
    void *instance,
    void *listener,
    const bool_t with_run_loop,
    FPtr_app_call func_OnFinishLaunching,
    FPtr_app_call func_OnTimerSignal);

void *_osapp_init_pool(void);

void _osapp_release_pool(void *pool);

void *_osapp_listener_imp(void);

void _osapp_terminate_imp(
    OSApp **app,
    const bool_t abnormal_termination,
    FPtr_destroy func_destroy,
    FPtr_app_void func_OnExecutionEnd);

uint32_t _osapp_argc_imp(OSApp *app);

uint32_t _osapp_argv_imp(OSApp *app, const uint32_t index, char_t *argv, const uint32_t max_size);

void _osapp_run(OSApp *app);

void _osapp_request_user_attention(OSApp *app);

void _osapp_cancel_user_attention(OSApp *app);

void *_osapp_begin_thread(OSApp *app);

void _osapp_end_thread(OSApp *app, void *data);

void _osapp_set_lang(OSApp *app, const char_t *lang);

void _osapp_OnThemeChanged(OSApp *app, Listener *listener);

void _osapp_menubar(OSApp *app, void *menu, void *window);

__END_C

#define _osapp_init(argc, argv, instance, listener, with_run_loop, func_OnFinishLaunching, func_OnTimerSignal, type) \
    ( \
        (void)(cast(listener, type) == listener), \
        FUNC_CHECK_APP_CALL(func_OnFinishLaunching, type), \
        FUNC_CHECK_APP_CALL(func_OnTimerSignal, type), \
        _osapp_init_imp( \
            argc, argv, instance, \
            cast(listener, void), \
            with_run_loop, \
            (FPtr_app_call)func_OnFinishLaunching, \
            (FPtr_app_call)func_OnTimerSignal))

#define _osapp_terminate(app, abnormal_termination, func_destroy, func_OnExecutionEnd, type) \
    ( \
        FUNC_CHECK_DESTROY(func_destroy, type), \
        FUNC_CHECK_APP_VOID(func_OnExecutionEnd), \
        _osapp_terminate_imp( \
            app, abnormal_termination, \
            (FPtr_destroy)func_destroy, \
            (FPtr_app_void)func_OnExecutionEnd))

#define _osapp_listener(type) \
    cast(_osapp_listener_imp(), type)
