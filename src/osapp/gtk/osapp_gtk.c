/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osapp_gtk.c
 *
 */

/* Application runloop */

#include "osapp_gtk.inl"
#include "../osapp.h"
#include "../osapp.inl"
#include <osgui/osgui.h>
#include <core/event.h>
#include <core/hfile.h>
#include <core/strings.h>
#include <osbs/bfile.h>
#include <osbs/log.h>
#include <sewer/bmem.h>
#include <sewer/bstd.h>
#include <sewer/cassert.h>
#include <sewer/unicode.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/stat.h>

#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
#endif

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

struct _osapp_t
{
    GtkApplication *gtk_app;
    uint32_t argc;
    char_t **argv;
    gchar *resources_dir;
    gchar *user_dir;
    guint timer_loop_id;
    guint timer_init_id;
    void *listener;
    GdkPixbuf *icon;
    bool_t is_init;
    bool_t terminate;
    bool_t abnormal_termination;
    bool_t with_run_loop;
    Listener *OnTheme;
    FPtr_app_call func_OnFinishLaunching;
    FPtr_app_call func_OnTimerSignal;
    FPtr_destroy func_destroy;
    FPtr_app_void func_OnExecutionEnd;
};

/*---------------------------------------------------------------------------*/

static OSApp i_APP;

/*---------------------------------------------------------------------------*/

/* Reduce an arbitrary executable name to characters a GApplication id component allows */
static void i_sanitize_id(const char_t *name, char_t *dest, const uint32_t size)
{
    uint32_t i = 0, j = 0;
    for (i = 0; name[i] != 0 && j < size - 1; ++i)
    {
        char_t c = name[i];
        if (c >= 'A' && c <= 'Z')
            dest[j++] = (char_t)(c - 'A' + 'a');
        else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-')
            dest[j++] = c;
        else
            dest[j++] = '-';
    }

    dest[j] = 0;
}

/*---------------------------------------------------------------------------*/

/* GNOME/Wayland (and modern Mutter, even under X11) resolve an app's Dock/taskbar icon
   through its application id matched against an installed '.desktop' file -- the per-window
   pixbuf set later via gtk_window_set_icon() is only honored as a fallback by older desktop
   versions. Every NAppGUI app used to share one generic, non-unique, never-installed id
   ("com.nappgui.app"), so modern GNOME had nothing to resolve an icon from. Derive a unique
   id from the executable's own name instead, so a matching '.desktop' file (written in
   i_OnActivate(), once the icon path is known) can be found by the Dock. */
static void i_app_id(char_t *app_id, const uint32_t size)
{
    char_t pathname[1024];
    str_copy_c(app_id, size, "com.nappgui.app");
    if (bfile_dir_exec(pathname, sizeof(pathname)) > 0)
    {
        char_t sanitized[256];
        i_sanitize_id(str_filename(pathname), sanitized, sizeof(sanitized));
        if (sanitized[0] != 0)
        {
            char_t candidate[300];
            bstd_sprintf(candidate, sizeof(candidate), "com.nappgui.%s", sanitized);
            if (g_application_id_is_valid(candidate) == TRUE)
                str_copy_c(app_id, size, candidate);
        }
    }
}

/*---------------------------------------------------------------------------*/

/* Write a '.desktop' file to the user's own XDG applications directory (no root/install
   step needed) so GNOME's Dock/taskbar can resolve this app's icon through 'app_id',
   matching what a properly packaged/installed application would provide. */
static void i_write_desktop_file(const char_t *app_id, const char_t *exe_pathname, const char_t *icon_pathname)
{
    char_t home[512];
    if (bfile_dir_home(home, sizeof(home)) > 0)
    {
        String *dir = str_cpath("%s/.local/share/applications", home);
        ferror_t err = ekFOK;
        if (hfile_dir_create(tc(dir), &err) == TRUE)
        {
            String *path = str_cpath("%s/%s.desktop", tc(dir), app_id);
            String *content = str_printf(
                "[Desktop Entry]\n"
                "Type=Application\n"
                "Name=%s\n"
                "Exec=%s\n"
                "Icon=%s\n"
                "Terminal=false\n"
                "StartupWMClass=%s\n",
                str_filename(exe_pathname), exe_pathname, icon_pathname, app_id);
            hfile_from_string(tc(path), content, NULL);

            /* bfile_create() (used internally by hfile_from_string()) always creates files
               as 0777-before-umask -- harmless in general, but a '.desktop' file has no
               reason to be executable. Match the 0644 that CMake's own build-time copy of
               this same file already uses (prj/NAppDesktopFile.cmake, plain file(WRITE)). */
            chmod(tc(path), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            str_destroy(&content);
            str_destroy(&path);
        }

        str_destroy(&dir);
    }
}

/*---------------------------------------------------------------------------*/

OSApp *_osapp_init_imp(
    uint32_t argc,
    char_t **argv,
    void *instance,
    void *listener,
    const bool_t with_run_loop,
    FPtr_app_call func_OnFinishLaunching,
    FPtr_app_call func_OnTimerSignal)
{
    bmem_zero(&i_APP, OSApp);
    cassert_unref(instance == NULL, instance);
    cassert_no_null(listener);
    cassert_no_nullf(func_OnFinishLaunching);
    cassert(i_APP.listener == NULL);
    cassert(i_APP.func_OnFinishLaunching == NULL);
    cassert(i_APP.func_OnTimerSignal == NULL);
    {
        char_t app_id[300];
        i_app_id(app_id, sizeof(app_id));
        cassert(g_application_id_is_valid(app_id) == TRUE);
        i_APP.gtk_app = gtk_application_new(app_id, G_APPLICATION_NON_UNIQUE);

        /* Load the icon and write the '.desktop' file as early as possible -- before
           _osapp_run() ever calls g_application_run(), which registers 'app_id' with the
           desktop (D-Bus activation) before this app gets a chance to run any of its own
           "activate"-signal code. Doing this later left the very first launch of a demo
           without a Dock icon, since the desktop's app_id -> icon lookup happened before
           the '.desktop' file existed. */
        {
            char_t pathname[1024];
            if (bfile_dir_exec(pathname, sizeof(pathname)) < sizeof(pathname))
            {
                String *logo = str_cpath("%s.ico", pathname);
                i_APP.icon = gdk_pixbuf_new_from_file(tc(logo), NULL);
                osgui_set_app(i_APP.gtk_app, i_APP.icon);
                i_write_desktop_file(app_id, pathname, tc(logo));
                str_destroy(&logo);
            }
        }
    }
    cassert_no_null(i_APP.gtk_app);
    i_APP.argc = argc;
    i_APP.argv = argv;
    i_APP.resources_dir = g_strdup("resources_dir");
    i_APP.user_dir = g_strdup("user_dir");
    i_APP.timer_loop_id = 0;
    i_APP.timer_init_id = 0;
    i_APP.listener = listener;
    i_APP.is_init = FALSE;
    i_APP.terminate = FALSE;
    i_APP.abnormal_termination = FALSE;
    i_APP.with_run_loop = with_run_loop;
    i_APP.OnTheme = NULL;
    i_APP.func_OnFinishLaunching = func_OnFinishLaunching;
    i_APP.func_OnTimerSignal = func_OnTimerSignal;
    return &i_APP;
}

/*---------------------------------------------------------------------------*/

void *_osapp_init_pool(void)
{
    return NULL;
}

/*---------------------------------------------------------------------------*/

void _osapp_release_pool(void *pool)
{
    cassert_unref(pool == NULL, pool);
}

/*---------------------------------------------------------------------------*/

void *_osapp_listener_imp(void)
{
    return i_APP.listener;
}

/*---------------------------------------------------------------------------*/

static void i_terminate(OSApp *app)
{
    cassert_no_null(app);
    cassert(app == &i_APP);
    cassert_no_nullf(app->func_destroy);
    cassert_no_nullf(app->func_OnExecutionEnd);
    cassert(app->terminate == TRUE);
    osgui_terminate();

    if (app->abnormal_termination == FALSE)
    {
        g_free((gpointer)app->resources_dir);
        g_free((gpointer)app->user_dir);
        app->func_destroy(&app->listener);
    }

    if (app->icon != NULL)
        g_object_unref(app->icon);

    listener_destroy(&app->OnTheme);
    g_application_quit(G_APPLICATION(app->gtk_app));
}

/*---------------------------------------------------------------------------*/

void _osapp_terminate_imp(OSApp **app, const bool_t abnormal_termination, FPtr_destroy func_destroy, FPtr_app_void func_OnExecutionEnd)
{
    cassert_no_null(app);
    cassert_no_null(*app);
    cassert(*app == &i_APP);
    cassert((*app)->func_destroy == NULL);
    cassert((*app)->func_OnExecutionEnd == NULL);
    cassert_no_nullf(func_destroy);
    cassert_no_nullf(func_OnExecutionEnd);
    (*app)->abnormal_termination = abnormal_termination;
    (*app)->func_destroy = func_destroy;
    (*app)->func_OnExecutionEnd = func_OnExecutionEnd;
    (*app)->terminate = TRUE;
}

/*---------------------------------------------------------------------------*/

uint32_t _osapp_argc_imp(OSApp *app)
{
    cassert_no_null(app);
    cassert(app == &i_APP);
    return app->argc;
}

/*---------------------------------------------------------------------------*/

uint32_t _osapp_argv_imp(OSApp *app, const uint32_t index, char_t *argv, const uint32_t max_size)
{
    cassert_no_null(app);
    cassert(app == &i_APP);
    cassert(index < app->argc);
    return unicode_convers(cast_const(app->argv[index], char_t), argv, ekUTF8, ekUTF8, max_size);
}

/*---------------------------------------------------------------------------*/

/* This function will be called repeatedly during the life-cycle of the app
   until it returns FALSE, at which point the timeout is automatically destroyed
   and the function will not be called again. */
static gboolean i_OnTimerLoop(gpointer data)
{
    OSApp *app = (OSApp *)data;
    cassert(app == &i_APP);

    if (app->terminate == TRUE)
    {
        i_terminate(app);
        return FALSE;
    }

    if (app->func_OnTimerSignal != NULL)
        app->func_OnTimerSignal(app->listener);

    return TRUE;
}

/*---------------------------------------------------------------------------*/

/* This function will be called repeatedly until the application is created,
   at which point the timeout is automatically destroyed and the function will
   not be called again. */
static gboolean i_OnTimerInit(gpointer data)
{
    OSApp *app = (OSApp *)data;
    cassert(app == &i_APP);

    /* Create impostor window before app running */
    if (osgui_is_initialized() == TRUE)
    {
        if (app->is_init == FALSE)
        {
            app->is_init = TRUE;
            app->func_OnFinishLaunching(app->listener);
            return FALSE;
        }
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*/

static void i_OnActivate(GtkApplication *gtk_app, OSApp *app)
{
    cassert_no_null(app);
    cassert_no_null(app->listener);
    cassert_no_nullf(app->func_OnTimerSignal);
    cassert(app->timer_loop_id == 0);
    cassert(app->timer_init_id == 0);

    /* printf decimal separator */
    setlocale(LC_NUMERIC, "C");

    osgui_initialize();
    g_application_hold(G_APPLICATION(gtk_app));

    app->timer_loop_id = g_timeout_add(10, i_OnTimerLoop, (gpointer)app);
    app->timer_init_id = g_timeout_add(10, i_OnTimerInit, (gpointer)app);
    cassert(app->timer_loop_id > 0);
    cassert(app->timer_init_id > 0);

    {
        GdkDisplay *display = gdk_display_get_default();
        const char_t *type_name = cast_const(g_type_name(G_TYPE_FROM_INSTANCE(display)), char_t);
        const char_t *backend = type_name;
#ifdef GDK_WINDOWING_WAYLAND
        if (GDK_IS_WAYLAND_DISPLAY(display) == TRUE)
            backend = "Wayland";
#endif
        if (str_str(type_name, "X11") != NULL)
            backend = "X11";

        log_printf("GTK3 %s backend", backend);
    }
}

/*---------------------------------------------------------------------------*/

void _osapp_run(OSApp *app)
{
    gulong signal_id = 0;
    int status = 0;
    cassert_no_null(app);
    cassert(app->with_run_loop == TRUE);
    signal_id = g_signal_connect(app->gtk_app, "activate", G_CALLBACK(i_OnActivate), (gpointer)app);
    cassert_unref(signal_id > 0, signal_id);
    status = g_application_run(G_APPLICATION(app->gtk_app), 0, NULL);
    unref(status);
    app->func_OnExecutionEnd();
}

/*---------------------------------------------------------------------------*/

void _osapp_request_user_attention(OSApp *app)
{
    unref(app);
}

/*---------------------------------------------------------------------------*/

void _osapp_cancel_user_attention(OSApp *app)
{
    unref(app);
}

/*---------------------------------------------------------------------------*/

void *_osapp_begin_thread(OSApp *app)
{
    unref(app);
    return NULL;
}

/*---------------------------------------------------------------------------*/

void _osapp_end_thread(OSApp *app, void *data)
{
    unref(app);
    unref(data);
}

/*---------------------------------------------------------------------------*/

void osapp_open_url(const char_t *url)
{
    String *cmd = str_printf("x-www-browser %s", url);
    int t = system(tc(cmd));
    str_destroy(&cmd);
    unref(t);
}

/*---------------------------------------------------------------------------*/

void _osapp_set_lang(OSApp *app, const char_t *lang)
{
    unref(app);
    unref(lang);
}

/*---------------------------------------------------------------------------*/

void _osapp_OnThemeChanged(OSApp *app, Listener *listener)
{
    cassert_no_null(app);
    listener_update(&app->OnTheme, listener);
}

/*---------------------------------------------------------------------------*/

void _osapp_menubar(OSApp *app, void *menu, void *window)
{
    GMenuModel *gmenu = NULL;
    cassert_no_null(app);
    unref(window);
    gmenu = gtk_application_get_menubar(app->gtk_app);
    /* In some Linux/GTK distros, the menubar is attached to common menubar, not in the window itself */
    if (menu == NULL && gmenu != NULL)
        gtk_application_set_menubar(app->gtk_app, NULL);
}
