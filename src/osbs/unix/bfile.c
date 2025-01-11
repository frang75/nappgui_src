/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bfile.c
 *
 */

/* Basic file system services */

#include "../bfile.h"
#include "../osbs.inl"
#include <sewer/bmem.h>
#include <sewer/blib.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>
#include <sewer/unicode.h>

#if !defined(__UNIX__)
#error This file is for Unix/Unix-like system
#endif

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#if defined(__MACOS__)
#include <sys/syslimits.h>
#endif

#if defined(__LINUX__)
#include <sys/types.h>
#include <time.h>
int lstat(const char *path, struct stat *buf);

#ifndef DTTOIF
#define DTTOIF(dirtype) ((dirtype) << 12)
#endif

#endif

static const uint32_t i_NUM_RETRYS = 10;

/* Not used, only for debugger inspection */
struct _dir_t
{
    DIR *dir;
    uint32_t pathsize;
    char_t pathname[256];
};

/*---------------------------------------------------------------------------*/

uint32_t bfile_dir_work(char_t *pathname, const uint32_t size)
{
    const char *buff = getcwd(pathname, size);
    if (buff != NULL)
        return (uint32_t)(strlen(pathname) + 1);
    return 0;
}

/*---------------------------------------------------------------------------*/

bool_t bfile_dir_set_work(const char_t *pathname, ferror_t *error)
{
    cassert_no_null(pathname);
    if (chdir(cast_const(pathname, char)) == 0)
    {
        ptr_assign(error, ekFOK);
        return TRUE;
    }
    else
    {
        if (error != NULL)
        {
            switch (errno)
            {
            case EACCES:
                *error = ekFNOACCESS;
                break;
            case EEXIST:
                *error = ekFEXISTS;
                break;
            case ENAMETOOLONG:
                *error = ekFBIGNAME;
                break;
            case ENOENT:
                *error = ekFNOPATH;
                break;
            default:
                cassert_msg(FALSE, "dir_set_work: undefined");
                *error = ekFUNDEF;
            }
        }

        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

uint32_t bfile_dir_tmp(char_t *pathname, const uint32_t size)
{
    const char_t *temp = "/tmp";
    uint32_t s = (uint32_t)strlen(temp);
    if (s > size - 1)
        s = size - 1;
    blib_strncpy(pathname, size, temp, s);
    pathname[s] = '\0';
    return s;
}

/*---------------------------------------------------------------------------*/

bool_t bfile_dir_create(const char_t *pathname, ferror_t *error)
{
    int res = mkdir(cast_const(pathname, char), (mode_t)(S_IRUSR | S_IWUSR | S_IXUSR));
    if (res == 0)
    {
        ptr_assign(error, ekFOK);
        return TRUE;
    }
    else
    {
        if (error != NULL)
        {
            switch (errno)
            {
            case EACCES:
                *error = ekFNOACCESS;
                break;
            case EEXIST:
                *error = ekFEXISTS;
                break;
            case ENAMETOOLONG:
                *error = ekFBIGNAME;
                break;
            case ENOENT:
                *error = ekFNOPATH;
                break;
            default:
                cassert_msg(FALSE, "dir_create: undefined");
                *error = ekFUNDEF;
            }
        }

        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

#define i_PATHNAME(dir) (char *)(cast(dir, char_t) + sizeof(DIR *) + sizeof(uint32_t))

/*---------------------------------------------------------------------------*/

Dir *bfile_dir_open(const char_t *pathname, ferror_t *error)
{
    DIR *dir = opendir(cast_const(pathname, char));
    if (dir != NULL)
    {
        uint32_t pathsize = (uint32_t)strlen(cast_const(pathname, char)) + 2;
        Dir *ldir = cast(bmem_malloc(pathsize + sizeof32(uint32_t) + sizeof32(DIR *)), Dir);
        ldir->dir = dir;
        ldir->pathsize = pathsize;
        strcpy(i_PATHNAME(ldir), cast_const(pathname, char));
        strcat(i_PATHNAME(ldir), "/");
        _osbs_directory_alloc();
        ptr_assign(error, ekFOK);
        return ldir;
    }
    else
    {
        if (error != NULL)
        {
            switch (errno)
            {
            case EACCES:
                *error = ekFNOACCESS;
                break;
            case ENOENT:
                *error = ekFNOPATH;
                break;
            case ENAMETOOLONG:
                *error = ekFBIGNAME;
                break;
            default:
                cassert_msg(FALSE, "dir_open: undefined");
                *error = ekFUNDEF;
            }
        }

        return NULL;
    }
}

/*---------------------------------------------------------------------------*/

void bfile_dir_close(Dir **dir)
{
    int res;
    cassert_no_null(dir);
    cassert_no_null(*dir);

    do
    {
        res = closedir((*dir)->dir);
        if (res == -1)
        {
            cassert(errno == EINTR);
        }

    } while (res == -1);

    bmem_free(cast(*dir, byte_t));
    _osbs_directory_dealloc();
    *dir = NULL;
}

/*---------------------------------------------------------------------------*/

static int i_file_mode(const file_mode_t mode)
{
    switch (mode)
    {
    case ekREAD:
        return O_RDONLY;
    case ekWRITE:
        return O_WRONLY /* | O_CREAT | O_TRUNC*/;
    case ekAPPEND:
        return O_WRONLY;
        cassert_default();
    }

    return -1;
}

/*---------------------------------------------------------------------------*/

static file_type_t i_file_type(const mode_t mode)
{
    if (S_ISREG(mode) == TRUE)
        return ekARCHIVE;
    else if (S_ISDIR(mode) == TRUE)
        return ekDIRECTORY;
    else if (S_ISLNK(mode) == TRUE)
        return ekOTHERFILE;
    else if (S_ISCHR(mode) == TRUE)
        return ekOTHERFILE;
    else if (S_ISBLK(mode) == TRUE)
        return ekOTHERFILE;
    else if (S_ISFIFO(mode) == TRUE)
        return ekOTHERFILE;
#if defined(S_ISSOCK)
    else if (S_ISSOCK(mode) == TRUE)
        return ekOTHERFILE;
#endif
    else
        return ekOTHERFILE;
}

/*---------------------------------------------------------------------------*/

static void i_file_date(time_t *rawtime, Date *date)
{
    /* Copy from btime_get_current_date() */
    struct tm *timeinfo;
    timeinfo = localtime(rawtime);
    cassert_no_null(timeinfo);
    cassert_no_null(date);
    date->wday = (uint8_t)timeinfo->tm_wday;
    date->mday = (uint8_t)timeinfo->tm_mday;
    date->month = (uint8_t)(1 + timeinfo->tm_mon);
    date->year = 1900 + (int16_t)timeinfo->tm_year;
    date->hour = (uint8_t)timeinfo->tm_hour;
    date->minute = (uint8_t)timeinfo->tm_min;
    date->second = (uint8_t)timeinfo->tm_sec;
}

/*---------------------------------------------------------------------------*/

bool_t bfile_dir_get(Dir *dir, char_t *name, const uint32_t size, file_type_t *file_type, uint64_t *file_size, Date *last_update, ferror_t *error)
{
    int current_errno = errno;
    const struct dirent *dirent = NULL;
    cassert_no_null(dir);
    dirent = readdir(dir->dir);
    if (dirent != NULL)
    {
        if (name != NULL)
        {
            uint32_t nb = unicode_convers(cast_const(dirent->d_name, char_t), name, ekUTF8, ekUTF8, size);
            if (nb == size)
            {
                ptr_assign(error, ekFBIGNAME);
                return FALSE;
            }
        }

        if (file_type != NULL)
            *file_type = i_file_type((mode_t)DTTOIF(dirent->d_type));

        if (file_size != NULL || last_update != NULL)
        {
            struct stat info;
            char pathname[512];
            strcpy(pathname, i_PATHNAME(dir));
            strcat(pathname, dirent->d_name);
            if (lstat(pathname, &info) == 0)
            {
                if (file_size != NULL)
                    *file_size = (uint64_t)info.st_size;

                if (last_update != NULL)
                {
                    time_t rawtime;
#if defined(__MACOS__)
                    rawtime = (time_t)info.st_mtimespec.tv_sec;
#else
                    rawtime = info.st_mtime;
#endif
                    i_file_date(&rawtime, last_update);
                }
            }
            else
            {
                *file_size = 0;
            }
        }

        ptr_assign(error, ekFOK);
        return TRUE;
    }
    else
    {
        if (error != NULL)
        {
            if (errno == current_errno)
            {
                *error = ekFNOFILES;
            }
            else
            {
                cassert_msg(FALSE, "dir_get: undefined");
                *error = ekFUNDEF;
            }
        }

        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

bool_t bfile_dir_delete(const char_t *pathname, ferror_t *error)
{
    int result = rmdir(cast_const(pathname, char));
    if (result == 0)
    {
        ptr_assign(error, ekFOK);
        return TRUE;
    }
    else
    {
        if (error != NULL)
        {
            switch (errno)
            {
            case EACCES:
                *error = ekFNOACCESS;
                break;
            case EEXIST:
            case ENOTEMPTY:
                *error = ekFNOEMPTY;
                break;
            case ENOENT:
                *error = ekFNOPATH;
                break;
            case ENAMETOOLONG:
                *error = ekFBIGNAME;
                break;
            default:
                cassert_msg(FALSE, "dir_remove: undefined");
                *error = ekFUNDEF;
            }
        }

        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

static File *i_after_open_file(int file_id, ferror_t *error)
{
    if (file_id > 0)
    {
        _osbs_file_alloc();
        ptr_assign(error, ekFOK);
        return cast((intptr_t)file_id, File);
    }
    else
    {
        if (error != NULL)
        {
            int e = errno;
            switch (e)
            {
            case EACCES:
                *error = ekFNOACCESS;
                break;
            case EEXIST:
                *error = ekFEXISTS;
                break;
            case ENOENT:
                *error = ekFNOPATH;
                break;
            case ENAMETOOLONG:
                *error = ekFBIGNAME;
                break;
            case ENOTDIR:
                *error = ekFNOPATH;
                break;
            case EISDIR:
                *error = ekFNOFILE;
                break;
            default:
                cassert_msg(FALSE, "file_open: undefined");
                *error = ekFUNDEF;
            }
        }

        return NULL;
    }
}

/*---------------------------------------------------------------------------*/

File *bfile_create(const char_t *filepath, ferror_t *error)
{
    int file_id = creat(cast_const(filepath, char), (mode_t)(0777) /*(S_IRUSR | S_IWUSR | S_IXUSR)*/);
    return i_after_open_file(file_id, error);
}

/*---------------------------------------------------------------------------*/

File *bfile_open(const char_t *filepath, const file_mode_t mode, ferror_t *error)
{
    int file_id = open(cast_const(filepath, char), i_file_mode(mode), 0);
    File *file = i_after_open_file(file_id, error);
    if (file != NULL && mode == ekAPPEND)
        lseek(file_id, 0, SEEK_END);
    return file;
}

/*---------------------------------------------------------------------------*/

void bfile_close(File **file)
{
    int ret = 0;

    cassert_no_null(file);
    cassert_no_null(*file);

    do
    {
        ret = close((int)(intptr_t)*file);

        if (ret == -1)
        {
            cassert(errno == EINTR);
        }

    } while (ret == -1);

    _osbs_file_dealloc();
    *file = NULL;
}

/*---------------------------------------------------------------------------*/

static bool_t i_file_stat(const int err, const struct stat *info, file_type_t *file_type, uint64_t *file_size, Date *last_update, ferror_t *error)
{
    if (err == 0)
    {
        cassert_no_null(info);

        if (file_type != NULL)
            *file_type = i_file_type(info->st_mode);

        if (file_size != NULL)
            *file_size = (uint64_t)info->st_size;

        if (last_update != NULL)
        {
            time_t rawtime;
#if defined(__MACOS__)
            rawtime = (time_t)info->st_mtimespec.tv_sec;
#else
            rawtime = info->st_mtime;
#endif
            i_file_date(&rawtime, last_update);
        }

        ptr_assign(error, ekFOK);
        return TRUE;
    }
    else
    {
        if (error != NULL)
        {
            switch (errno)
            {
            case EACCES:
                *error = ekFNOACCESS;
                break;
            case EEXIST:
                *error = ekFEXISTS;
                break;
            case ENOENT:
                *error = ekFNOPATH;
                break;
            case ENAMETOOLONG:
                *error = ekFBIGNAME;
                break;
            case ENOTDIR:
                *error = ekFNOPATH;
                break;
            default:
                cassert_msg(FALSE, "file_stat: undefined");
                *error = ekFUNDEF;
            }
        }
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

bool_t bfile_lstat(const char_t *filepath, file_type_t *file_type, uint64_t *file_size, Date *last_update, ferror_t *error)
{
    int ret;
    struct stat info;
    uint32_t i = 0;
    while ((ret = lstat(cast_const(filepath, char), &info)) != 0)
    {
        i++;
        if (i == i_NUM_RETRYS)
            break;
    }

    return i_file_stat(ret, &info, file_type, file_size, last_update, error);
}

/*---------------------------------------------------------------------------*/

bool_t bfile_fstat(const File *file, file_type_t *file_type, uint64_t *file_size, Date *last_update, ferror_t *error)
{
    int ret = 0;
    struct stat info;
    cassert_no_null(file);
    ret = fstat((int)(intptr_t)file, &info);
    return i_file_stat(ret, &info, file_type, file_size, last_update, error);
}

/*---------------------------------------------------------------------------*/

bool_t bfile_read(File *file, byte_t *data, const uint32_t size, uint32_t *rsize, ferror_t *error)
{
    ssize_t lrsize;
    cassert_no_null(file);
    lrsize = read((int)(intptr_t)file, cast(data, void), (size_t)size);
    if (lrsize > 0)
    {
        ptr_assign(rsize, (uint32_t)lrsize);
        ptr_assign(error, ekFOK);
        return TRUE;
    }
    else if (lrsize == 0)
    {
        ptr_assign(rsize, 0);
        ptr_assign(error, ekFOK);
        return FALSE;
    }
    else
    {
        cassert_msg(FALSE, "file_read: undefined");
        ptr_assign(error, ekFUNDEF);
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

bool_t bfile_write(File *file, const byte_t *data, const uint32_t size, uint32_t *wsize, ferror_t *error)
{
    int fd = (int)(intptr_t)file;
    ssize_t lwsize = 0;
    cassert_no_null(file);
    lwsize = write(fd, cast_const(data, void), (size_t)size);
    if (lwsize >= 0)
    {
        ptr_assign(wsize, (uint32_t)lwsize);
        ptr_assign(error, ekFOK);
        return TRUE;
    }
    else
    {
        cassert_msg(FALSE, "file_write: undefined");
        ptr_assign(error, ekFUNDEF);
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

bool_t bfile_seek(File *file, const int64_t offset, const file_seek_t whence, ferror_t *error)
{
    int fd = (int)(intptr_t)file;
    int method = 0;
    off_t res = 0;
    cassert_no_null(file);

    switch (whence)
    {
    case ekSEEKSET:
        method = SEEK_SET;
        break;
    case ekSEEKCUR:
        method = SEEK_CUR;
        break;
    case ekSEEKEND:
        method = SEEK_END;
        break;
        cassert_default();
    }

    res = lseek(fd, (off_t)offset, method);
    if (res == -1)
    {
        if (errno == EINVAL)
        {
            ptr_assign(error, ekFSEEKNEG);
        }
        else
        {
            ptr_assign(error, ekFUNDEF);
        }

        return FALSE;
    }

    ptr_assign(error, ekFOK);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

uint64_t bfile_pos(const File *file)
{
    int fd = (int)(intptr_t)file;
    cassert_no_null(file);
    return (uint64_t)lseek(fd, (off_t)0, SEEK_CUR);
}

/*---------------------------------------------------------------------------*/

bool_t bfile_delete(const char_t *filepath, ferror_t *error)
{
    int res = unlink(cast_const(filepath, char));
    if (res == 0)
    {
        ptr_assign(error, ekFOK);
        return TRUE;
    }
    else
    {
        if (error != NULL)
        {
            switch (errno)
            {
            case EACCES:
                *error = ekFNOACCESS;
                break;
            case EEXIST:
                *error = ekFEXISTS;
                break;
            case ENOENT:
                *error = ekFNOPATH;
                break;
            case ENAMETOOLONG:
                *error = ekFBIGNAME;
                break;
            case ENOTDIR:
                *error = ekFNOPATH;
                break;
            default:
                cassert_msg(FALSE, "file_delete: undefined");
                *error = ekFUNDEF;
            }
        }

        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

bool_t bfile_rename(const char_t *current_pathname, const char_t *new_pathname, ferror_t *error)
{
    int res = rename(cast_const(current_pathname, char), cast_const(new_pathname, char));
    if (res == 0)
    {
        ptr_assign(error, ekFOK);
        return TRUE;
    }
    else
    {
        if (error != NULL)
        {
            switch (errno)
            {
            case EACCES:
            case EPERM:
                *error = ekFNOACCESS;
                break;
            case EISDIR:
                *error = ekFNOFILE;
                break;
            case ENAMETOOLONG:
                *error = ekFBIGNAME;
                break;
            case ENOENT:
                *error = ekFNOPATH;
                break;
            case ENOTDIR:
                *error = ekFNOPATH;
                break;
            case EEXIST:
            case ENOTEMPTY:
                *error = ekFNOEMPTY;
                break;

            /* https://man7.org/linux/man-pages/man2/rename.2.html */
            case EROFS:
            case EXDEV:
            case EBUSY:
            case EFAULT:
            case EINVAL:
            case ELOOP:
            case EMLINK:
            case ENOMEM:
            case ENOSPC:
            default:
                cassert_msg(FALSE, "file_rename: undefined");
                *error = ekFUNDEF;
            }
        }

        return FALSE;
    }
}
