#include "../../meshfs.h"
#include <thread>

#define FUSE_USE_VERSION 26

#include <fuse/fuse.h>
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
        .getattr = test_fuse::getattr,
        .readlink = nullptr,//test_fuse::readlink,
        .getdir = nullptr, // deprecated
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
        .utime = nullptr,//test_fuse::utime,
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
        .ftruncate = nullptr,//test_fuse::ftruncate,
        .fgetattr = nullptr,//test_fuse::fgetattr
    };

    struct test_data
    {
        std::shared_ptr<meshfs<MYDEVID> > mfs;
        std::string name;
    };

    static void mfs_file_info_default(fuse_file_info* fi)
    {
        fi->direct_io = true;
        fi->keep_cache = false;
        fi->nonseekable = true;
    };
}

int test_fuse::getattr(const char *path, struct stat *statbuf)
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
}

int test_fuse::readlink(const char *path, char *link, size_t size)
{

}

int test_fuse::mknod(const char *path, mode_t mode, dev_t dev)
{
    // TODO
}

int test_fuse::mkdir(const char *path, mode_t mode)
{

}

int test_fuse::unlink(const char *path)
{
    // TODO
}

int test_fuse::rmdir(const char *path)
{

}

int test_fuse::symlink(const char *path, const char *link)
{

}

int test_fuse::rename(const char *path, const char *newpath)
{

}

int test_fuse::link(const char *path, const char *newpath)
{

}

int test_fuse::chmod(const char *path, mode_t mode)
{

}

int test_fuse::chown(const char *path, uid_t uid, gid_t gid)
{

}

int test_fuse::truncate(const char *path, off_t newsize)
{

}

int test_fuse::utime(const char *path, struct utimbuf *ubuf)
{

}

int test_fuse::open(const char *path, struct fuse_file_info *fi)
{
    // TODO
}

int test_fuse::read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    // TODO
}

int test_fuse::write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    // TODO
}

int test_fuse::statfs(const char *path, struct statvfs *statv)
{
    // TODO
}

int test_fuse::flush(const char *path, struct fuse_file_info *fi)
{

}

int test_fuse::release(const char *path, struct fuse_file_info *fi)
{
    // TODO
}

int test_fuse::fsync(const char *path, int datasync, struct fuse_file_info *fi)
{

}

#ifdef HAVE_SYS_XATTR_H
int test_fuse::setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{

}

int test_fuse::getxattr(const char *path, const char *name, char *value, size_t)
{

}

int test_fuse::listxattr(const char *path, char *list, size_t size)
{

}

int test_fuse::removexattr(const char *path, const char *name)
{

}
#endif

int test_fuse::opendir(const char *path, struct fuse_file_info *fi)
{
    // TODO
}

int test_fuse::readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    // TODO
}

int test_fuse::fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{

}

int test_fuse::releasedir(const char *path, struct fuse_file_info *fi)
{
    // TODO
}

void *test_fuse::init(struct fuse_conn_info *conn)
{
    std::cout << "Log: (init)" << std::endl;
    TEST_DATA->mfs.reset(new meshfs<MYDEVID>(TEST_DATA->name));

    return TEST_DATA;
}

void test_fuse::destroy(void *userdata)
{
    delete TEST_DATA;
}

int test_fuse::access(const char *path, int mask)
{

}

int test_fuse::ftruncate(const char *path, off_t offset, struct fuse_file_info *fi)
{

}

int test_fuse::fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{

}

void mythread(char **argv, std::string name)
{
    std::cout << "Log: (mythread)\n" << std::flush;
    test_fuse::test_data *data = new test_fuse::test_data;
    data->name = name;

    std::cout << "Path: " << argv[1] << " Name: " << name << '\n' << std::flush;

    int mfs_stat = fuse_main(2, argv, &test_fuse::mfs_ops, data);
}

int main(int argc, char *argv[])
{
    if ((getuid() == 0) || (geteuid() == 0)) {
    	std::cerr << "Running this test as root opens unnacceptable security holes" << std::endl;
    	return 1;
    }

    char *mfs1_path[2] = {"test","./mfs1"};
    std::string mfs1_name = "mfs1";

    char *mfs2_path[2] = {"test","./mfs2"};
    std::string mfs2_name = "mfs2";

    std::thread mfs1(mythread, mfs1_path, mfs1_name);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::thread mfs2(mythread, mfs2_path, mfs2_name);
    mfs1.join();
    mfs2.join();
    return 0;
}