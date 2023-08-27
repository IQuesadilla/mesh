#ifndef MESHFS_FUSE_H
#define MESHFS_FUSE_H
#pragma once

#define FUSE_USE_VERSION 26

#include "filemesh.h"
#include <fuse/fuse.h>
#include <unistd.h>
#include <iostream>
#if __has_include (<sys/xattr.h>)
    #include <sys/xattr.h>
    #define HAVE_SYS_XATTR_H
#endif

#define MFS_DATA ((mfs_fuse::mfs_data *) fuse_get_context()->private_data)

namespace mfs_fuse
{
    int getattr(const char *path, struct stat *statbuf);
    int readlink(const char *path, char *link, size_t size);
    int mknod(const char *path, mode_t mode, dev_t dev);
    int mkdir(const char *path, mode_t mode);
    int unlink(const char *path);
    int rmdir(const char *path);
    int symlink(const char *path, const char *link);
    int rename(const char *path, const char *newpath);
    int link(const char *path, const char *newpath);
    int chmod(const char *path, mode_t mode);
    int chown(const char *path, uid_t uid, gid_t gid);
    int truncate(const char *path, off_t newsize);
    int utime(const char *path, struct utimbuf *ubuf);
    int open(const char *path, fuse_file_info *fi);
    int read(const char *path, char *buf, size_t size, off_t offset, fuse_file_info *fi);
    int write(const char *path, const char *buf, size_t size, off_t offset, fuse_file_info *fi);
    int statfs(const char *path, struct statvfs *statv);
    int flush(const char *path, fuse_file_info *fi);
    int release(const char *path, fuse_file_info *fi);
    int fsync(const char *path, int datasync, fuse_file_info *fi);

    #ifdef HAVE_SYS_XATTR_H
    int setxattr(const char *path, const char *name, const char *value, size_t size, int flags);
    int getxattr(const char *path, const char *name, char *value, size_t);
    int listxattr(const char *path, char *list, size_t size);
    int removexattr(const char *path, const char *name);
    #endif

    int opendir(const char *path, fuse_file_info *fi);
    int readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, fuse_file_info *fi);
    int fsyncdir(const char *path, int datasync, fuse_file_info *fi);
    int releasedir(const char *path, fuse_file_info *fi);
    void *init(struct fuse_conn_info *conn);
    void destroy(void *userdata);
    int access(const char *path, int mask);
    int ftruncate(const char *path, off_t offset, fuse_file_info *fi);
    int fgetattr(const char *path, struct stat *statbuf, fuse_file_info *fi);

    static fuse_operations mfs_ops = {
        .getattr = mfs_fuse::getattr,
        .readlink = nullptr,//mfs_fuse::readlink,
        .getdir = nullptr, // deprecated
        .mknod = mfs_fuse::mknod,
        .mkdir = nullptr,//mfs_fuse::mkdir,
        .unlink = mfs_fuse::unlink,
        .rmdir = nullptr,//mfs_fuse::rmdir,
        .symlink = nullptr,//mfs_fuse::symlink,
        .rename = nullptr,//mfs_fuse::rename,
        .link = nullptr,//mfs_fuse::link,
        .chmod = nullptr,//mfs_fuse::chmod,
        .chown = nullptr,//mfs_fuse::chown,
        .truncate = nullptr,//mfs_fuse::truncate,
        .utime = nullptr,//mfs_fuse::utime,
        .open = mfs_fuse::open,
        .read = mfs_fuse::read,
        .write = mfs_fuse::write,
        /** Just a placeholder, don't set */ // huh???
        .statfs = mfs_fuse::statfs,
        .flush = nullptr,//mfs_fuse::flush,
        .release = mfs_fuse::release,
        .fsync = nullptr,//mfs_fuse::fsync,
        
        #ifdef HAVE_SYS_XATTR_H
        .setxattr = nullptr,//mfs_fuse::setxattr,
        .getxattr = nullptr,//mfs_fuse::getxattr,
        .listxattr = nullptr,//mfs_fuse::listxattr,
        .removexattr = nullptr,//mfs_fuse::removexattr,
        #endif
        
        .opendir = mfs_fuse::opendir,
        .readdir = mfs_fuse::readdir,
        .releasedir = mfs_fuse::releasedir,
        .fsyncdir = nullptr,//mfs_fuse::fsyncdir,
        .init = mfs_fuse::init,
        .destroy = mfs_fuse::destroy,
        .access = nullptr,//mfs_fuse::access,
        .ftruncate = nullptr,//mfs_fuse::ftruncate,
        .fgetattr = nullptr,//mfs_fuse::fgetattr
    };

    struct mfs_data
    {
        filemesh *fuse_filemesh;
    };

    static void mfs_file_info_default(fuse_file_info* fi)
    {
        fi->direct_io = true;
        fi->keep_cache = false;
        fi->nonseekable = true;
    };
}
#endif