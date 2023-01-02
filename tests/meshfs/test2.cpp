#include "../../meshfs.h"
#include <thread>
#include <fstream>
#include <cstdlib>

#define FUSE_USE_VERSION 30

#include <fuse3/fuse.h>
#include <unistd.h>
#include <iostream>
#if __has_include (<sys/xattr.h>)
    #include <sys/xattr.h>
    #define HAVE_SYS_XATTR_H
#endif

#define MYDEVID std::string
bool firstname = true;

#define TEST_DATA ((test_fuse::test_data *) fuse_get_context()->private_data)

namespace test_fuse
{
    int getattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi);
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
    int readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, fuse_file_info *fi, fuse_readdir_flags flags);
    int fsyncdir(const char *path, int datasync, fuse_file_info *fi);
    int releasedir(const char *path, fuse_file_info *fi);
    void *init(struct fuse_conn_info *conn, struct fuse_config *cfg);
    void destroy(void *userdata);
    int access(const char *path, int mask);
    int ftruncate(const char *path, off_t offset, fuse_file_info *fi);
    int fgetattr(const char *path, struct stat *statbuf, fuse_file_info *fi);

    static fuse_operations mfs_ops = {
        .getattr = test_fuse::getattr,
        .readlink = nullptr,//test_fuse::readlink,
        .mknod = test_fuse::mknod,
        .mkdir = nullptr,//test_fuse::mkdir,
        .unlink = test_fuse::unlink,
        .rmdir = nullptr,//test_fuse::rmdir,
        .symlink = nullptr,//test_fuse::symlink,
        .rename = nullptr,//test_fuse::rename,
        .link = nullptr,//test_fuse::link,
        .chmod = nullptr,//test_fuse::chmod,
        .chown = nullptr,//test_fuse::chown,
        .truncate = nullptr,//test_fuse::truncate,
//        .utime = nullptr,//test_fuse::utime,
        .open = test_fuse::open,
        .read = test_fuse::read,
        .write = test_fuse::write,
        /** Just a placeholder, don't set */ // huh???
        .statfs = test_fuse::statfs,
        .flush = nullptr,//test_fuse::flush,
        .release = test_fuse::release,
        .fsync = nullptr,//test_fuse::fsync,
        
        #ifdef HAVE_SYS_XATTR_H
        .setxattr = nullptr,//test_fuse::setxattr,
        .getxattr = nullptr,//test_fuse::getxattr,
        .listxattr = nullptr,//test_fuse::listxattr,
        .removexattr = nullptr,//test_fuse::removexattr,
        #endif
        
        .opendir = test_fuse::opendir,
        .readdir = test_fuse::readdir,
        .releasedir = test_fuse::releasedir,
        .fsyncdir = nullptr,//test_fuse::fsyncdir,
        .init = test_fuse::init,
        .destroy = test_fuse::destroy,
        .access = nullptr,//test_fuse::access,
        //.ftruncate = nullptr,//test_fuse::ftruncate,
        //.fgetattr = nullptr,//test_fuse::fgetattr
    };

    struct test_data
    {
        std::shared_ptr<meshfs<MYDEVID> > mfs;
        std::string name;
        std::fstream tlog;
    };

    static void mfs_file_info_default(fuse_file_info* fi)
    {
        fi->direct_io = true;
        fi->keep_cache = false;
        fi->nonseekable = true;
    };
}

int test_fuse::getattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
    statbuf->st_atim = {0};
    statbuf->st_blksize = 64;
    statbuf->st_blocks = 0;
    statbuf->st_ctim = {0};
    statbuf->st_dev = 0;
    statbuf->st_gid = 0;
    statbuf->st_ino = 0;
    statbuf->st_mode = 0;
    statbuf->st_mtim = {0};
    statbuf->st_nlink = 1;
    statbuf->st_rdev = 0;
    statbuf->st_size = 1000;
    statbuf->st_uid = 0;
    return 0;
}

int test_fuse::readlink(const char *path, char *link, size_t size)
{
    return ENOTSUP;
}

int test_fuse::mknod(const char *path, mode_t mode, dev_t dev)
{
    // TODO
    return ENOTSUP;
}

int test_fuse::mkdir(const char *path, mode_t mode)
{
    return ENOTSUP;
}

int test_fuse::unlink(const char *path)
{
    // TODO
    return ENOTSUP;
}

int test_fuse::rmdir(const char *path)
{
    return ENOTSUP;
}

int test_fuse::symlink(const char *path, const char *link)
{
    return ENOTSUP;
}

int test_fuse::rename(const char *path, const char *newpath)
{
    return ENOTSUP;
}

int test_fuse::link(const char *path, const char *newpath)
{
    return ENOTSUP;
}

int test_fuse::chmod(const char *path, mode_t mode)
{
    return ENOTSUP;
}

int test_fuse::chown(const char *path, uid_t uid, gid_t gid)
{
    return ENOTSUP;
}

int test_fuse::truncate(const char *path, off_t newsize)
{
    return ENOTSUP;
}

int test_fuse::utime(const char *path, struct utimbuf *ubuf)
{
    return ENOTSUP;
}

int test_fuse::open(const char *path, struct fuse_file_info *fi)
{
    // TODO
    return ENOTSUP;
}

int test_fuse::read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    // TODO
    return ENOTSUP;
}

int test_fuse::write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    // TODO
    return ENOTSUP;
}

int test_fuse::statfs(const char *path, struct statvfs *statv)
{
    // TODO
    return ENOTSUP;
}

int test_fuse::flush(const char *path, struct fuse_file_info *fi)
{
    return ENOTSUP;
}

int test_fuse::release(const char *path, struct fuse_file_info *fi)
{
    // TODO
    return ENOTSUP;
}

int test_fuse::fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
    return ENOTSUP;
}

#ifdef HAVE_SYS_XATTR_H
int test_fuse::setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{
    return ENOTSUP;
}

int test_fuse::getxattr(const char *path, const char *name, char *value, size_t)
{
    return ENOTSUP;
}

int test_fuse::listxattr(const char *path, char *list, size_t size)
{
    return ENOTSUP;
}

int test_fuse::removexattr(const char *path, const char *name)
{
    return ENOTSUP;
}
#endif

int test_fuse::opendir(const char *path, struct fuse_file_info *fi)
{
    // TODO
    return ENOTSUP;
}

int test_fuse::readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, fuse_readdir_flags flags)
{
    // TODO
    return ENOTSUP;
}

int test_fuse::fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{
    return ENOTSUP;
}

int test_fuse::releasedir(const char *path, struct fuse_file_info *fi)
{
    // TODO
    return ENOTSUP;
}

void *test_fuse::init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    TEST_DATA->tlog << "Log: (init)" << std::endl;
    TEST_DATA->mfs.reset(new meshfs<MYDEVID>(TEST_DATA->name));

    return TEST_DATA;
}

void test_fuse::destroy(void *userdata)
{
    delete TEST_DATA;
}

int test_fuse::access(const char *path, int mask)
{
    return ENOTSUP;
}

int test_fuse::ftruncate(const char *path, off_t offset, struct fuse_file_info *fi)
{
    return ENOTSUP;
}

int test_fuse::fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
    return ENOTSUP;
}

int main(int argc, char *argv[])
{
    if ((getuid() == 0) || (geteuid() == 0)) {
    	std::cerr << "Running this test as root opens unnacceptable security holes" << std::endl;
    	return 1;
    }

    char *newpath[2] = {argv[1],argv[1]};

    test_fuse::test_data *data = new test_fuse::test_data;
    data->name = newpath[0];

    data->tlog.open ("/tmp/mfslog.txt", std::fstream::out);
    data->tlog << "Log: (mythread)" << std::endl;
    data->tlog << "Path: " << std::string(newpath[1]) << " Name: " << newpath[0] << std::endl;

    int mfs_stat = fuse_main(2, newpath, &test_fuse::mfs_ops, data);
    return 0;
}