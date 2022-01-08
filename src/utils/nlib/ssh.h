/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ssh.h
 *
 */

/* SSH Commands */
    
#include "nlib.hxx"

bool_t ssh_ping(const char_t *ip);

uint32_t ssh_repo_version(const Login *login, const char_t *repo_url);

uint32_t ssh_working_version(const Login *login, const char_t *path);

String *ssh_working_version2(const char_t *path, const char_t *type);

Stream *ssh_repo_list(const Login *login, const char_t *repo_url, const uint32_t repo_vers);

Stream *ssh_repo_cat(const Login *login, const char_t *repo_url, const uint32_t repo_vers);

bool_t ssh_repo_export(const Login *login, const char_t *repo_url, const uint32_t repo_vers, const char_t *dest);

bool_t ssh_repo_checkout(const Login *login, const char_t *repo_url, const char_t *user, const char_t *pass, const uint32_t repo_vers, const char_t *dest);

bool_t ssh_dir_exists(const Login *login, const char_t *path);

bool_t ssh_file_exists(const Login *login, const char_t *path, const char_t *filename);

bool_t ssh_create_dir(const Login *login, const char_t *dir);

bool_t ssh_empty_dir(const Login *login, const char_t *dir);

bool_t ssh_create_file(const Login *login, const char_t *file, const char_t *content);

bool_t ssh_delete_dir(const Login *login, const char_t *path);

Stream *ssh_file_cat(const Login *login, const char_t *path, const char_t *filename);

bool_t ssh_to_file(const Login *login, const Stream *stm, const char_t *remote_path, const char_t *filename);

bool_t ssh_copy_to_remote(const Login *login, const char_t *from_local, const char_t *to_remote);

bool_t ssh_copy_from_remote(const Login *login, const char_t *from_remote, const char_t *to_local);

bool_t ssh_copy(const Login *from_login, const char_t *from_path, const char_t *from_filename, const Login *to_login, const char_t *to_path, const char_t *to_filename);

Stream *ssh_cmake(const Login *login, const char_t *cmake_command, const char_t *src_dir, const char_t *build_dir, bool_t *ok);

Stream *ssh_msbuild(const Login *login, const char_t *msbuild_command, const char_t *build_dir);

Stream *ssh_msbuild_ver(const Login *login, const char_t *msbuild_path);

Stream *ssh_vcbuild_ver(const Login *login, const char_t *vcbuild_path);

Stream *ssh_xcode_ver(const Login *login);

bool_t ssh_xcode_reset(const Login *login);

bool_t ssh_xcode_select(const Login *login, const char_t *xcode_path);

Stream *ssh_xcode_build(const Login *login, const char_t *xcode_command, const char_t *build_dir);

bool_t ssh_gcc_reset(const Login *login);

bool_t ssh_gcc_select(const Login *login, const char_t *gcc_path);

Stream *ssh_make(const Login *login, const char_t *make_command, const char_t *build_dir);
