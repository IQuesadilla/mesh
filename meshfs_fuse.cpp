#include "meshfs_fuse.h"

int mfs_fuse::getattr(const char *path, struct stat *statbuf)
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

int mfs_fuse::readlink(const char *path, char *link, size_t size)
{

}

int mfs_fuse::mknod(const char *path, mode_t mode, dev_t dev)
{
    // TODO
}

int mfs_fuse::mkdir(const char *path, mode_t mode)
{

}

int mfs_fuse::unlink(const char *path)
{
    // TODO
}

int mfs_fuse::rmdir(const char *path)
{

}

int mfs_fuse::symlink(const char *path, const char *link)
{

}

int mfs_fuse::rename(const char *path, const char *newpath)
{

}

int mfs_fuse::link(const char *path, const char *newpath)
{

}

int mfs_fuse::chmod(const char *path, mode_t mode)
{

}

int mfs_fuse::chown(const char *path, uid_t uid, gid_t gid)
{

}

int mfs_fuse::truncate(const char *path, off_t newsize)
{

}

int mfs_fuse::utime(const char *path, struct utimbuf *ubuf)
{

}

int mfs_fuse::open(const char *path, struct fuse_file_info *fi)
{
    // TODO
}

int mfs_fuse::read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    // TODO
}

int mfs_fuse::write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    // TODO
}

int mfs_fuse::statfs(const char *path, struct statvfs *statv)
{
    // TODO
}

int mfs_fuse::flush(const char *path, struct fuse_file_info *fi)
{

}

int mfs_fuse::release(const char *path, struct fuse_file_info *fi)
{
    // TODO
}

int mfs_fuse::fsync(const char *path, int datasync, struct fuse_file_info *fi)
{

}

#ifdef HAVE_SYS_XATTR_H
int mfs_fuse::setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{

}

int mfs_fuse::getxattr(const char *path, const char *name, char *value, size_t)
{

}

int mfs_fuse::listxattr(const char *path, char *list, size_t size)
{

}

int mfs_fuse::removexattr(const char *path, const char *name)
{

}
#endif

int mfs_fuse::opendir(const char *path, struct fuse_file_info *fi)
{
    // TODO
}

int mfs_fuse::readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    // TODO
}

int mfs_fuse::fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{

}

int mfs_fuse::releasedir(const char *path, struct fuse_file_info *fi)
{
    // TODO
}

void *mfs_fuse::init(struct fuse_conn_info *conn)
{
    mfs_fuse::mfs_data *data = new mfs_fuse::mfs_data;

    data->fuse_filemesh = new filemesh();
    data->fuse_filemesh->connect("myname");

    return data;
}

void mfs_fuse::destroy(void *userdata)
{
    MFS_DATA->fuse_filemesh->disconnect();
    delete MFS_DATA;
}

int mfs_fuse::access(const char *path, int mask)
{

}

int mfs_fuse::ftruncate(const char *path, off_t offset, struct fuse_file_info *fi)
{

}

int mfs_fuse::fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{

}

int main(int argc, char *argv[])
{
    if ((getuid() == 0) || (geteuid() == 0)) {
    	std::cerr << "Running MeshFS as root opens unnacceptable security holes" << std::endl;
    	return 1;
    }

    int mfs_stat = fuse_main(argc, argv, &mfs_fuse::mfs_ops, nullptr);
}