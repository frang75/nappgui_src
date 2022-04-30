/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ssh.c
 *
 */

/* SSH Commands */
    
#include "ssh.h"
#include "bproc.h"
#include "bthread.h"
#include "cassert.h"
#include "hfile.h"
#include "log.h"
#include "osbs.h"
#include "ptr.h"
#include "stream.h"
#include "strings.h"

typedef struct _proc_std_t ProcStd;

struct _proc_std_t
{
    Proc *proc;
    Stream *stm;
};

#define READ_BUFFER_SIZE    1024

/*---------------------------------------------------------------------------*/

static __INLINE bool_t i_localhost(const Login *login)
{
    if (login == NULL)
        return TRUE;    
    return str_equ(login->ip, "localhost");
}

/*---------------------------------------------------------------------------*/

static uint32_t i_std_err(ProcStd *data)
{
    byte_t buffer[READ_BUFFER_SIZE + 1];
    uint32_t rsize;
    cassert_no_null(data);

    while (bproc_eread(data->proc, buffer, READ_BUFFER_SIZE, &rsize, NULL) == TRUE)
        stm_write(data->stm, buffer, rsize);
    
    return 0;
}

/*---------------------------------------------------------------------------*/
//#include"log.h"
#include"bstd.h"
static Stream *i_ssh_command(const Login *login, const char_t *cmd, const char_t *ssh_opts, const bool_t capture_stderr, uint32_t *return_value)
{
    String *ssh = NULL;
    Proc *proc = NULL;
    Stream *stm = NULL;
    ptr_assign(return_value, UINT32_MAX);

    if (!i_localhost(login))
    {
		char_t qt = osbs_platform() == ekWINDOWS ? '\"' : '\'';
        if (ssh_opts != NULL)
            ssh = str_printf("ssh %s %s@%s %c%s%c", ssh_opts, tc(login->user), tc(login->ip), qt, cmd, qt);
        else
            ssh = str_printf("ssh %s@%s %c%s%c", tc(login->user), tc(login->ip), qt, cmd, qt);
    }
    else
    {
        ssh = str_c(cmd);
    }

    //log_printf(tc(ssh));
    proc = bproc_exec(tc(ssh), NULL);
    if (proc != NULL)
    {
        ProcStd proc_err = {NULL, NULL};
        Thread *thread_err = NULL;
        byte_t buffer[READ_BUFFER_SIZE + 1];
        uint32_t rsize, ret;

        stm = stm_memory(READ_BUFFER_SIZE);

        if (capture_stderr == TRUE)
        {
            proc_err.proc = proc;
            proc_err.stm = stm_memory(READ_BUFFER_SIZE);
            thread_err = bthread_create(i_std_err, &proc_err, ProcStd);
        }
        else
        {
            bproc_eread_close(proc);
        }

        while(bproc_read(proc, buffer, READ_BUFFER_SIZE, &rsize, NULL) == TRUE)
        {
            //buffer[rsize] = '\0';
            //bstd_printf("stdout %d: %s", rsize, buffer);
            stm_write(stm, buffer, rsize);
        }

        ret = bproc_wait(proc);

        if (thread_err != NULL)
            bthread_wait(thread_err);

        bproc_close(&proc);

        if (proc_err.stm != NULL)
        {
            stm_writef(stm, "\nstderr:\n");
            stm_pipe(proc_err.stm, stm, stm_buffer_size(proc_err.stm));
            stm_close(&proc_err.stm);
        }

        ptr_assign(return_value, ret);
        // ssh 0 success
        //if (ret != 0)
        //    stm_close(&stm);
    }

    str_destroy(&ssh);	
    return stm;
}

/*---------------------------------------------------------------------------*/

static bool_t i_ssh_ok(const Login *login, String **cmd, const char_t *ssh_opts)
{
    Stream *stm = i_ssh_command(login, tc(*cmd), ssh_opts, FALSE, NULL);
    str_destroy(cmd);
    if (stm != NULL)
    {
        stm_close(&stm);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

bool_t ssh_ping(const char_t *ip)
{
    String *cmd = NULL;
    Proc *proc = NULL;
    bool_t ok = FALSE;
    if (osbs_platform() == ekWINDOWS)
        cmd = str_printf("ping %s -n 1", ip);
    else
        cmd = str_printf("ping %s -c 1", ip);

    proc = bproc_exec(tc(cmd), NULL);
    if (proc != NULL)
    {
        Stream *stm = NULL;
        String *str = NULL;
        byte_t buffer[READ_BUFFER_SIZE];
        uint32_t rsize;
        stm = stm_memory(READ_BUFFER_SIZE);
        while (bproc_read(proc, buffer, READ_BUFFER_SIZE, &rsize, NULL) == TRUE)
            stm_write(stm, buffer, rsize);
        bproc_close(&proc);

        str = stm_str(stm);
        str_lower(str);
        if (str_str(tc(str), "host unreachable") == NULL)
            ok = TRUE;
        str_destroy(&str);
        stm_close(&stm);
    }

    str_destroy(&cmd);
    return ok;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_repo_version(const Login *login, const char_t *cmd)
{
    Stream *stm = i_ssh_command(login, cmd, NULL, FALSE, NULL);
    uint32_t ret = UINT32_MAX;

    if (stm != NULL)
    {
        const char_t *line = stm_read_line(stm);
        if (line)
        {
            ret = str_to_u32(line, 10, NULL);
            if (ret == 0)
                ret = UINT32_MAX;
        }
        stm_close(&stm);
    }

    return ret;
}

/*---------------------------------------------------------------------------*/

static String *i_repo_version2(const Login *login, const char_t *cmd)
{
    Stream *stm = i_ssh_command(login, cmd, NULL, FALSE, NULL);
    String *ret = NULL;

    if (stm != NULL)
    {
        const char_t *line = stm_read_line(stm);
        if (line)
            ret = str_c(line);
        stm_close(&stm);
    }

    return ret;
}

/*---------------------------------------------------------------------------*/

//static uint32_t i_repo_version_3(const Login *login, const char_t *cmd)
//{
//    Stream *stm = i_ssh_command(login, cmd, NULL, FALSE, NULL);
//    uint32_t ret = UINT32_MAX;
//
//    if (stm != NULL)
//    {
//        stm_lines(line, stm)
//            if (str_is_prefix(line, "Revision") 
//                || str_is_prefix(line, "revision")
//                || str_is_prefix(line, "Revisión"))
//            {
//                String *number = NULL;
//                str_split(line, ":", NULL, &number);
//                ret = str_to_u32(tc(number), 10, NULL);
//                if (ret == 0)
//                    ret = UINT32_MAX;
//
//                str_destroy(&number);
//                break;
//            }
//        stm_next(line, stm);
//
//        stm_close(&stm);
//    }
//
//    return ret;
//}

/*---------------------------------------------------------------------------*/

static uint32_t i_repo_version_4(const Login *login, const char_t *cmd)
{
    Stream *stm = i_ssh_command(login, cmd, NULL, FALSE, NULL);
    uint32_t ret = UINT32_MAX;

    if (stm != NULL)
    {
        const char_t *line = stm_read_line(stm);
        if (line)
        {
            String *number = NULL;
            if (str_str(line, ":") != NULL)
                str_split(line, ":", NULL, &number);
            else
                number = str_c(line);

            str_subs(number, 'M', '\0');
            str_subs(number, 'S', '\0');
            str_subs(number, 'P', '\0');

            ret = str_to_u32(tc(number), 10, NULL);
            str_destroy(&number);
        }

        stm_close(&stm);
    }

    return ret;
}

/*---------------------------------------------------------------------------*/

uint32_t ssh_repo_version(const Login *login, const char_t *repo_url)
{
    String *cmd = str_printf("svn info --show-item revision %s -r HEAD", repo_url);
    uint32_t vers = i_repo_version(login, tc(cmd));
    str_destroy(&cmd);
    return vers;
}

/*---------------------------------------------------------------------------*/

uint32_t ssh_working_version(const Login *login, const char_t *path)
{
    //String *cmd = str_printf("svn info --show-item revision %s", path);
    //String *cmd = str_printf("svn info %s", path);
    //uint32_t vers = i_repo_version_3(login, tc(cmd));
    String *cmd = str_printf("svnversion %s", path);
    uint32_t vers = i_repo_version_4(login, tc(cmd));
    str_destroy(&cmd);
    return vers;
}

/*---------------------------------------------------------------------------*/

String *ssh_working_version2(const char_t *path, const char_t *type)
{
    String *version = NULL;
    if (str_equ_c(type, "svn"))
    {
        String *cmd = str_printf("svn info --show-item revision %s", path);
        version = i_repo_version2(NULL, tc(cmd));
        str_destroy(&cmd);
    }
    else if (str_equ_c(type, "git"))
    {
        String *cmd = str_printf("git -C \"%s\" rev-parse --short HEAD", path);
        version = i_repo_version2(NULL, tc(cmd));
        str_destroy(&cmd);
    }
    else
    {
        version = str_c(type);
    }
    
    if (version == NULL)
        version = str_c("");

    return version;
}

/*---------------------------------------------------------------------------*/

Stream *ssh_repo_list(const Login *login, const char_t *repo_url, const uint32_t repo_vers)
{
    String *cmd = str_printf("svn list %s -r %d", repo_url, repo_vers);
    Stream *stm = i_ssh_command(login, tc(cmd), NULL, FALSE, NULL);
    str_destroy(&cmd);
    return stm;
}

/*---------------------------------------------------------------------------*/

Stream *ssh_repo_cat(const Login *login, const char_t *repo_url, const uint32_t repo_vers)
{
    String *cmd = str_printf("svn cat %s -r %d", repo_url, repo_vers);
    Stream *stm = i_ssh_command(login, tc(cmd), NULL, FALSE, NULL);
    str_destroy(&cmd);
    return stm;
}

/*---------------------------------------------------------------------------*/

bool_t ssh_repo_export(const Login *login, const char_t *repo_url, const uint32_t repo_vers, const char_t *dest)
{
    String *cmd = str_printf("svn export %s %s -r %d", repo_url, dest, repo_vers);
    return i_ssh_ok(login, &cmd, NULL);
}

/*---------------------------------------------------------------------------*/

bool_t ssh_repo_checkout(const Login *login, const char_t *repo_url, const char_t *user, const char_t *pass, const uint32_t repo_vers, const char_t *dest)
{
    //svn checkout svn://192.168.1.2/svn/NAPPGUI/1_0 C:/wctest -r 1000
    String *cmd = str_printf("svn checkout --username %s --password %s --non-interactive %s %s -r %d", user, pass, repo_url, dest, repo_vers);
    return i_ssh_ok(login, &cmd, NULL);
}

/*---------------------------------------------------------------------------*/

static bool_t i_exists(const Login *login, const char_t *file, const file_type_t type)
{
    bool_t ok = FALSE;
    String *cmd = NULL;
    Stream *stm = NULL;
    cassert_no_null(login);

    if (login->platform == ekMACOS || login->platform == ekLINUX)
    {
        cmd = str_printf("[ ! %s %s ] && echo NOT_EXISTS", type == ekARCHIVE ? "-f" : "-d", file);
    }
    else
    {
        cmd = str_printf("IF EXIST %s (echo Yes) ELSE (echo NOT_EXISTS)", file);
        str_subs(cmd, '/', '\\');
    }

    stm = i_ssh_command(login, tc(cmd), NULL, FALSE, NULL);
    if (stm != NULL)
    {
        const char_t *line = stm_read_line(stm);
        if (line == NULL || str_str(line, "NOT_EXISTS") == NULL)
            ok = TRUE;
        stm_close(&stm);
    }

    str_destroy(&cmd);
    return ok;
}

/*---------------------------------------------------------------------------*/

bool_t ssh_dir_exists(const Login *login, const char_t *path)
{
    return i_exists(login, path, ekDIRECTORY);
}

/*---------------------------------------------------------------------------*/

bool_t ssh_file_exists(const Login *login, const char_t *path, const char_t *filename)
{
    String *f = NULL;
    bool_t ok = FALSE;
    cassert_no_null(login);
    f = str_path(login->platform, "%s/%s", path, filename);
    ok = i_exists(login, tc(f), ekARCHIVE);
    str_destroy(&f);
    return ok;
}

/*---------------------------------------------------------------------------*/

static bool_t i_win_mkdir(const Login *login, char_t *path)
{
    bool_t ok = TRUE;
    if (ssh_dir_exists(login, path) == FALSE)
    {
        uint32_t size = str_len_c(path);
        uint32_t i = size;
        uint32_t dirs = 0;
        char_t sep[64];
        while (i > 0)
        {
            if (path[i] == '\\' || path[i] == '/')
            {
                sep[dirs] = path[i];
                path[i] = '\0';
                dirs += 1;
                if (ssh_dir_exists(login, path) == TRUE)
                    break;
            }

            i -= 1;
        }

        while (i < size)
        {
            if (path[i] == '\0')
            {
                cassert(dirs > 0);
                dirs -= 1;
                path[i] = sep[dirs];

                if (ok == TRUE)
                {
                    String *cmd = NULL;
                    Stream *stm = NULL;
                    cmd = str_printf("mkdir %s", path);
                    stm = i_ssh_command(login, tc(cmd), NULL, TRUE, NULL);
                    if (stm != NULL)
                    {
                        ptr_destopt(stm_close, &stm, Stream);
                    }
                    else
                    {
                        ok = FALSE;
                    }
                    str_destroy(&cmd);
                }
            }

            i += 1;
        }

        cassert(dirs == 0);
    }

    return ok;
}

/*---------------------------------------------------------------------------*/

bool_t ssh_create_dir(const Login *login, const char_t *dir)
{
    if (login->platform == ekWINDOWS)
    {
        return i_win_mkdir(login, (char_t*)dir);
    }
    else
    {
        String *cmd = str_printf("mkdir -p %s", dir);
        return i_ssh_ok(login, &cmd, NULL);
    }
}

/*---------------------------------------------------------------------------*/

bool_t ssh_empty_dir(const Login *login, const char_t *dir)
{
    bool_t ok = ssh_delete_dir(login, dir);
    bthread_sleep(500);
    ok &= ssh_create_dir(login, dir);
    return ok;
}

/*---------------------------------------------------------------------------*/

bool_t ssh_create_file(const Login *login, const char_t *file, const char_t *content)
{
    String *cmd = NULL;
    if (login->platform == ekWINDOWS)
    {
        cmd = str_printf("echo \"%s\" > %s", content, file);
    }
    else
    {
        cassert_msg(FALSE, "TODO:");
    }
    return i_ssh_ok(login, &cmd, NULL);
}

/*---------------------------------------------------------------------------*/

bool_t ssh_delete_dir(const Login *login, const char_t *path)
{
    String *cmd = NULL;
    cassert_no_null(login);
    if (login->platform == ekWINDOWS)
        cmd = str_printf("rd /s /q %s", path);
    else
        cmd = str_printf("rm -rf %s", path);
    return i_ssh_ok(login, &cmd, NULL);
}

/*---------------------------------------------------------------------------*/

Stream *ssh_file_cat(const Login *login, const char_t *path, const char_t *filename)
{
    String *cmd = NULL;
    Stream *stm = NULL;
    cassert_no_null(login);

    if (login->platform == ekMACOS || login->platform == ekLINUX)
        cmd = str_path(login->platform, "cat %s/%s", path, filename);
    else
        cmd = str_path(login->platform, "type %s/%s", path, filename);

    stm = i_ssh_command(login, tc(cmd), NULL, FALSE, NULL);
    str_destroy(&cmd);
    return stm;
}

/*---------------------------------------------------------------------------*/

bool_t ssh_to_file(const Login *login, const Stream *stm, const char_t *remote_path, const char_t *filename)
{
    const byte_t *data = stm_buffer(stm);
    uint32_t size = stm_buffer_size(stm);
    bool_t ok = FALSE;
    if (!i_localhost(login))
    {
        String *remote_file = str_path(login->platform, "%s/%s", remote_path, filename);
        String *tmp = hfile_appdata("temp_file");
        Stream *file = stm_to_file(tc(tmp), NULL);
        stm_write(file, data, size);
        stm_close(&file);
        ok = ssh_copy_to_remote(login, tc(tmp), tc(remote_file));
        str_destroy(&tmp);
        str_destroy(&remote_file);
    }
    else
    {
        String *remote_file = str_cpath("%s/%s", remote_path, filename);
        Stream *file = stm_to_file(tc(remote_file), NULL);
        if (file != NULL)
        {
            stm_write(file, data, size);
            stm_close(&file);
            ok = TRUE;
        }
        else
        {
            ok = FALSE;
        }

        str_destroy(&remote_file);
    }

    return ok;
}

/*---------------------------------------------------------------------------*/

bool_t ssh_copy_to_remote(const Login *login, const char_t *from_local, const char_t *to_remote)
{
    bool_t ok = FALSE;

    if (!i_localhost(login))
    {
		char_t qt = osbs_platform() == ekWINDOWS ? '\"' : '\'';
		String *cmd = str_printf("scp %c%s%c %c%s@%s:%s%c", qt, from_local, qt, qt, tc(login->user), tc(login->ip), to_remote, qt);
        Proc *proc = bproc_exec(tc(cmd), NULL);
        if (proc != NULL)
        {
            uint32_t ret = bproc_wait(proc);
            bproc_close(&proc);
            if (ret == 0)
                ok = TRUE;
        }

        str_destroy(&cmd);
    }
    else
    {
        ok = hfile_copy(from_local, to_remote, NULL);
    }

    return ok;
}

/*---------------------------------------------------------------------------*/

bool_t ssh_copy_from_remote(const Login *login, const char_t *from_remote, const char_t *to_local)
{
    bool_t ok = FALSE;
    cassert_no_null(login);
    if (!i_localhost(login))
    {
		char_t qt = osbs_platform() == ekWINDOWS ? '\"' : '\'';
		String *cmd = str_printf("scp %c%s@%s:%s%c %c%s%c", qt, tc(login->user), tc(login->ip), from_remote, qt, qt, to_local, qt);
        //log_printf(tc(cmd));
        Proc *proc = bproc_exec(tc(cmd), NULL);
        if (proc != NULL)
        {
            uint32_t ret = bproc_wait(proc);
            bproc_close(&proc);
            if (ret == 0)
                ok = TRUE;
        }

        str_destroy(&cmd);
    }
    else
    {
        ok = hfile_copy(from_remote, to_local, NULL);
    }

    return ok;
}

/*---------------------------------------------------------------------------*/

bool_t ssh_copy(const Login *from_login, const char_t *from_path, const char_t *from_filename, const Login *to_login, const char_t *to_path, const char_t *to_filename)
{
    bool_t ok = FALSE;
    String *tmp = hfile_appdata("temp_file");
    String *from = str_printf("%s/%s", from_path, from_filename);
    String *to = str_printf("%s/%s", to_path, to_filename);
    ok = ssh_copy_from_remote(from_login, tc(from), tc(tmp));
    if (ok == TRUE)
        ok = ssh_copy_to_remote(to_login, tc(tmp), tc(to));
    str_destroy(&tmp);
    str_destroy(&from);
    str_destroy(&to);
    return ok;
}

/*---------------------------------------------------------------------------*/

Stream *ssh_cmake(const Login *login, const char_t *cmake_command, const char_t *src_dir, const char_t *build_dir, bool_t *ok)
{
    String *cmd = NULL;
    Stream *stm = NULL;
    String *relpath = NULL;
    uint32_t ret;
    cassert_no_null(login);
    cassert_no_null(ok);
    relpath = str_relpath(build_dir, src_dir);
    if (login->platform == ekWINDOWS)
        cmd = str_printf("cd %s & %s %s", build_dir, cmake_command, tc(relpath));
    else
        cmd = str_printf("cd %s ; %s %s", build_dir, cmake_command, tc(relpath));

    stm = i_ssh_command(login, tc(cmd), NULL, TRUE, &ret);
    *ok = ret == 0 ? TRUE : FALSE;
    str_destroy(&cmd);
    str_destroy(&relpath);
    return stm;
}

/*---------------------------------------------------------------------------*/

Stream *ssh_msbuild(const Login *login, const char_t *msbuild_command, const char_t *build_dir)
{
    String *cmd = NULL;
    Stream *stm = NULL;
    cassert_no_null(login);
    cassert(login->platform == ekWINDOWS);
    cmd = str_printf("cd %s & %s", build_dir, msbuild_command);
    stm = i_ssh_command(login, tc(cmd), NULL, TRUE, NULL);
    str_destroy(&cmd);
    return stm;
}

/*---------------------------------------------------------------------------*/

Stream *ssh_msbuild_ver(const Login *login, const char_t *msbuild_path)
{
    String *cmd = NULL;
    Stream *stm = NULL;
    cassert_no_null(login);
    cassert(login->platform == ekWINDOWS);
    cmd = str_printf("\\\"%s\\\" /ver", msbuild_path);
    stm = i_ssh_command(login, tc(cmd), NULL, FALSE, NULL);
    str_destroy(&cmd);
    return stm;
}

/*---------------------------------------------------------------------------*/

Stream *ssh_vcbuild_ver(const Login *login, const char_t *vcbuild_path)
{
    String *cmd = NULL;
    Stream *stm = NULL;
    cassert_no_null(login);
    cassert(login->platform == ekWINDOWS);
    cmd = str_printf("\\\"%s\\\"", vcbuild_path);
    stm = i_ssh_command(login, tc(cmd), NULL, FALSE, NULL);
    str_destroy(&cmd);
    return stm;
}

/*---------------------------------------------------------------------------*/

Stream *ssh_xcode_ver(const Login *login)
{
    Stream *stm = NULL;
    cassert_no_null(login);
    cassert(login->platform == ekMACOS);
    stm = i_ssh_command(login, "xcodebuild -version", NULL, FALSE, NULL);
    return stm;
}

/*---------------------------------------------------------------------------*/

bool_t ssh_xcode_reset(const Login *login)
{
    String *cmd = str_printf("echo %s | sudo -S xcode-select --reset", tc(login->pass));
    return i_ssh_ok(login, &cmd, "-t");
}

/*---------------------------------------------------------------------------*/

bool_t ssh_xcode_select(const Login *login, const char_t *xcode_path)
{
    String *cmd = str_printf("echo %s | sudo -S xcode-select --switch %s/Xcode.app/Contents/Developer", tc(login->pass), xcode_path);
    return i_ssh_ok(login, &cmd, "-t");
}

/*---------------------------------------------------------------------------*/

Stream *ssh_xcode_build(const Login *login, const char_t *xcode_command, const char_t *build_dir)
{
    String *cmd = NULL;
    Stream *stm = NULL;
    cassert_no_null(login);
    cassert(login->platform == ekMACOS);
    cmd = str_printf("cd %s ; %s", build_dir, xcode_command);
    stm = i_ssh_command(login, tc(cmd), NULL, FALSE, NULL);
    str_destroy(&cmd);
    return stm;
}

/*---------------------------------------------------------------------------*/

bool_t ssh_gcc_reset(const Login *login)
{
    String *cmd = str_printf("echo %s | sudo -S update-alternatives --auto gcc", tc(login->pass));
    return i_ssh_ok(login, &cmd, "-t");
}

/*---------------------------------------------------------------------------*/

bool_t ssh_gcc_select(const Login *login, const char_t *gcc_path)
{
    String *cmd = str_printf("echo %s | sudo -S update-alternatives --set %s gcc", tc(login->pass), gcc_path);
    return i_ssh_ok(login, &cmd, "-t");
}

/*---------------------------------------------------------------------------*/

Stream *ssh_make(const Login *login, const char_t *make_command, const char_t *build_dir)
{
    String *cmd = NULL;
    Stream *stm = NULL;
    cassert_no_null(login);
    cassert(login->platform == ekLINUX);
    cmd = str_printf("cd %s ; %s", build_dir, make_command);
    stm = i_ssh_command(login, tc(cmd), NULL, TRUE, NULL);
    str_destroy(&cmd);
    return stm;
}
