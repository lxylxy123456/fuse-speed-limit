/*
 *	fuse-speed-limit - A FUSE file system that limits speed
 *	Copyright (C) 2026  lxylxy123456
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPLv2.
  See the file COPYING.
*/

/** @file
 *
 * minimal example filesystem using high-level API
 *
 * Compile with:
 *
 *     gcc -Wall hello.c `pkg-config fuse3 --cflags --libs` -o hello
 *
 * ## Source code ##
 * \include hello.c
 */

#define FUSE_USE_VERSION 31

#define _GNU_SOURCE

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>

#include <file_list.h>
#include <hello_ioctl.h>

/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */
static struct options {
	int show_help;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

typedef enum path_type {
	PATH_TYPE_ROOT,		// "/"
	PATH_TYPE_DIR,		// "/1000"
	PATH_TYPE_FILE,		// "/1000/abc"
	PATH_TYPE_NONE,		// "/invalid"
} path_type_t;

static path_type_t parse_path(const char *path, size_t *speed)
{
	*speed = 0;
	assert(path[0] == '/');
	path += 1;

	// "/".
	if (path[0] == '\0') {
		return PATH_TYPE_ROOT;
	}

	int count;
	int result = sscanf(path, "%zu%n", speed, &count);
	if (result != 1) {
		return PATH_TYPE_NONE;
	}
	path += count;

	// "/1000".
	if (path[0] == '\0') {
		return PATH_TYPE_DIR;
	}
	// "/1000/abc".
	if (path[0] == '/' && strchr(path + 1, '/') == NULL) {
		return PATH_TYPE_FILE;
	}

	return PATH_TYPE_NONE;
}

static void hello_sleep(size_t size, size_t speed)
{
	if (speed == 0) {
		return;
	}
	time_t s = size / speed;
	size_t rem = size % speed;
	size_t remn = rem * 1000000000;
	assert(remn / 1000000000 == rem);
	unsigned int ns = remn / speed;
	struct timespec duration = {.tv_sec = s,.tv_nsec = ns };
	while (nanosleep(&duration, &duration)) {
		assert(errno == EINTR);
	}
}

static void *hello_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
	cfg->direct_io = 1;

	/* Test setting flags the old way */
	fuse_set_feature_flag(conn, FUSE_CAP_ASYNC_READ);
	fuse_unset_feature_flag(conn, FUSE_CAP_ASYNC_READ);

	{
		struct timespec res;
		assert(clock_getres(CLOCK_REALTIME, &res) == 0);
		assert(res.tv_sec == 0);
		assert(res.tv_nsec == 1);
		conn->time_gran = res.tv_nsec;
	}

	srandom(time(NULL));

	return NULL;
}

static int hello_getattr(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	(void)fi;
	int res = 0;
	size_t speed;

	memset(stbuf, 0, sizeof(struct stat));
	switch (parse_path(path, &speed)) {
	case PATH_TYPE_ROOT:	/* fallthrough */
	case PATH_TYPE_DIR:
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
		break;
	case PATH_TYPE_FILE:
		stbuf->st_mode = S_IFREG | 0666;
		stbuf->st_nlink = 1;
		stbuf->st_size = 0;
		// Find size in file_list.
		{
			bool found = false;
			fl_lock();
			for (fl_item_t * i = fl_first(); i; i = fl_next(i)) {
				if (strcmp(path, i->path) == 0) {
					found = true;
					fli_lock(i);
					stbuf->st_size = i->size;
					stbuf->st_atim = i->atime;
					stbuf->st_mtim = i->mtime;
					stbuf->st_ctim = i->ctime;
					fli_unlock(i);
					break;
				}
			}
			fl_unlock();
			if (!found)
				return -ENOENT;
		}
		break;
	case PATH_TYPE_NONE:
		res = -ENOENT;
		break;
	default:
		assert(0);
		res = -ENOENT;
		break;
	}

	return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags)
{
	(void)offset;
	(void)fi;
	(void)flags;
	size_t speed;

	switch (parse_path(path, &speed)) {
	case PATH_TYPE_ROOT:
		filler(buf, ".", NULL, 0, FUSE_FILL_DIR_DEFAULTS);
		filler(buf, "..", NULL, 0, FUSE_FILL_DIR_DEFAULTS);
		fl_lock();
		for (fl_item_t * i = fl_first(); i; i = fl_next(i)) {
			bool duplicate = false;
			for (fl_item_t * j = fl_first(); j != i; j = fl_next(j))
				if (strcmp(i->dir, j->dir) == 0)
					duplicate = true;
			if (!duplicate)
				filler(buf, i->dir + 1, NULL, 0,
				       FUSE_FILL_DIR_DEFAULTS);
		}
		fl_unlock();
		return 0;
	case PATH_TYPE_DIR:
		filler(buf, ".", NULL, 0, FUSE_FILL_DIR_DEFAULTS);
		filler(buf, "..", NULL, 0, FUSE_FILL_DIR_DEFAULTS);
		fl_lock();
		for (fl_item_t * i = fl_first(); i; i = fl_next(i)) {
			if (strcmp(i->dir, path) != 0)
				continue;
			filler(buf, i->name, NULL, 0, FUSE_FILL_DIR_DEFAULTS);
		}
		fl_unlock();
		return 0;
	case PATH_TYPE_FILE:	/* fallthrough */
	case PATH_TYPE_NONE:
		return -ENOENT;
	default:
		assert(0);
		return -ENOENT;
	}
}

static int hello_create(const char *path, mode_t mode,
			struct fuse_file_info *fi)
{
	int res = 0;
	size_t speed;
	switch (parse_path(path, &speed)) {
	case PATH_TYPE_FILE:
		break;
	case PATH_TYPE_ROOT:	/* fallthrough */
	case PATH_TYPE_DIR:	/* fallthrough */
	case PATH_TYPE_NONE:
		return -ENOENT;
	default:
		assert(0);
		return -ENOENT;
	}

	fl_lock();
	for (fl_item_t * i = fl_first(); i; i = fl_next(i)) {
		if (strcmp(path, i->path) == 0) {
			res = -EEXIST;
			goto out;
		}
	}

	fl_item_t *fli = NULL;
	fli = fl_new(path);
	assert(fli);
	fli->size = 0;
	fli->speed = speed;

	fli->refcount++;
	assert(fli->refcount);
	fi->fh = (uint64_t) fli;

 out:
	fl_unlock();
	return res;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
	int res = 0;
	size_t speed;
	switch (parse_path(path, &speed)) {
	case PATH_TYPE_FILE:
		break;
	case PATH_TYPE_ROOT:	/* fallthrough */
	case PATH_TYPE_DIR:	/* fallthrough */
	case PATH_TYPE_NONE:
		return -ENOENT;
	default:
		assert(0);
		return -ENOENT;
	}

	fl_lock();
	fl_item_t *fli = NULL;
	for (fl_item_t * i = fl_first(); i; i = fl_next(i)) {
		if (strcmp(path, i->path) == 0) {
			fli = i;
			break;
		}
	}

	if (fli == NULL) {
		res = -ENOENT;
		goto out;
	}

	fli_lock(fli);
	if (fi->flags & O_TRUNC) {
		fli->size = 0;
	}

	fli->refcount++;
	assert(fli->refcount);
	fli_unlock(fli);
	fi->fh = (uint64_t) fli;

 out:
	fl_unlock();
	return res;
}

static int hello_release(const char *path, struct fuse_file_info *fi)
{
	fl_item_t *fli = (fl_item_t *) (fi->fh);
	fli_lock(fli);
	assert(strcmp(fli->path, path) == 0);
	assert(fli->refcount);
	fli->refcount--;
	assert(fli->refcount);
	fli_unlock(fli);
	return 0;
}

static int hello_rename(const char *oldpath, const char *newpath,
			unsigned int flags)
{
	int ret = 0;
	// Only supports renaming files.
	if (flags & RENAME_EXCHANGE) {
		return -EINVAL;
	}
	if (strcmp(oldpath, newpath) == 0) {
		return 0;
	}
	size_t speed;
	if (parse_path(newpath, &speed) != PATH_TYPE_FILE) {
		return -ENOENT;
	}
	fl_lock();
	fl_item_t *oldfli = NULL, *newfli = NULL;
	for (fl_item_t * i = fl_first(); i; i = fl_next(i)) {
		if (strcmp(i->path, oldpath) == 0) {
			assert(oldfli == NULL);
			oldfli = i;
		}
		if (strcmp(i->path, newpath) == 0) {
			assert(newfli == NULL);
			newfli = i;
		}
	}
	if (oldfli == NULL) {
		ret = -ENOENT;
		goto out;
	}
	assert(oldfli != newfli);
	if (newfli != NULL) {
		if (flags & RENAME_NOREPLACE) {
			ret = -EEXIST;
			goto out;
		} else {
			fl_free(newfli);
		}
	}
	fli_lock(oldfli);
	fl_free_path(oldfli);
	assert(fl_compute_path(oldfli, newpath) == 0);
	oldfli->speed = speed;
	assert(clock_gettime(CLOCK_REALTIME, &oldfli->ctime) == 0);
	fli_unlock(oldfli);
 out:
	fl_unlock();
	return ret;
}

static int hello_unlink(const char *path)
{
	int ret = 0;
	fl_lock();
	fl_item_t *fli = NULL;
	for (fl_item_t * i = fl_first(); i; i = fl_next(i)) {
		if (strcmp(path, i->path) == 0) {
			fli = i;
			break;
		}
	}
	if (fli) {
		fl_free(fli);
	} else {
		ret = -ENOENT;
	}
	fl_unlock();
	return ret;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	fl_item_t *fli = (fl_item_t *) (fi->fh);
#if !(HELLO_FAST & 1)
	fli_lock(fli);
	assert(strcmp(fli->path, path) == 0);
#endif				/* !(HELLO_FAST & 1) */

	size_t len = fli->size;
	size_t speed = fli->speed;
#if !(HELLO_FAST & 1)
	fli_unlock(fli);
#endif				/* !(HELLO_FAST & 1) */
	if (offset >= len)
		return 0;

	if (offset + size > len)
		size = len - offset;

	memset(buf, (int)(random()), size);

	hello_sleep(size, speed);

#if !(HELLO_FAST & 1)
	fli_lock(fli);
#endif				/* !(HELLO_FAST & 1) */
	assert(clock_gettime(CLOCK_REALTIME, &fli->atime) == 0);
	fli->read_loc = offset + size;
#if !(HELLO_FAST & 1)
	fli_unlock(fli);
#endif				/* !(HELLO_FAST & 1) */

	return size;
}

static int hello_write(const char *path, const char *buf, size_t size,
		       off_t offset, struct fuse_file_info *fi)
{
	(void)buf;
	fl_item_t *fli = (fl_item_t *) (fi->fh);
#if !(HELLO_FAST & 1)
	fli_lock(fli);
	assert(strcmp(fli->path, path) == 0);
#endif				/* !(HELLO_FAST & 1) */

	size_t speed = fli->speed;
#if !(HELLO_FAST & 1)
	fli_unlock(fli);
#endif				/* !(HELLO_FAST & 1) */

	hello_sleep(size, speed);

#if !(HELLO_FAST & 1)
	fli_lock(fli);
#endif				/* !(HELLO_FAST & 1) */
	size_t new_size = offset + size;
	if (fli->size < new_size)
		fli->size = new_size;
	assert(clock_gettime(CLOCK_REALTIME, &fli->mtime) == 0);
	fli->write_loc = new_size;
#if !(HELLO_FAST & 1)
	fli_unlock(fli);
#endif				/* !(HELLO_FAST & 1) */

	return size;
}

static int hello_truncate(const char *path, off_t size,
			  struct fuse_file_info *fi)
{
	fl_item_t *fli = (fl_item_t *) (fi->fh);
	fli_lock(fli);
	assert(strcmp(fli->path, path) == 0);

	fli->size = size;
	fli_unlock(fli);
	return 0;
}

static int hello_ioctl(const char *path, int cmd, void *arg,
		       struct fuse_file_info *fi, unsigned int flags,
		       void *data)
{
	if (flags & FUSE_IOCTL_COMPAT)
		return -ENOSYS;

	if (flags & FUSE_IOCTL_DIR)
		return -EINVAL;

	switch (cmd) {
	case HELLO_IOCTL_GETINFO:
		{
			hello_ioctl_getinfo_t *ans = data;
			fl_item_t *fli = (fl_item_t *) (fi->fh);
			ans->read_loc = fli->read_loc;
			ans->write_loc = fli->write_loc;
		}
		return 0;
	default:
		return -EINVAL;
	}
}

static const struct fuse_operations hello_oper = {
	.init = hello_init,
	.getattr = hello_getattr,
	.readdir = hello_readdir,
	.create = hello_create,
	.open = hello_open,
	.release = hello_release,
	.rename = hello_rename,
	.unlink = hello_unlink,
	.read = hello_read,
	.write = hello_write,
	.truncate = hello_truncate,
	.ioctl = hello_ioctl,
};

static void show_help(const char *progname)
{
	printf("usage: %s [options] <mountpoint>\n\n", progname);
	printf("File-system specific options:\n" "\n");
}

int main(int argc, char *argv[])
{
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	/* Parse options */
	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

	/* When --help is specified, first print our own file-system
	   specific help text, then signal fuse_main to show
	   additional help (by adding `--help` to the options again)
	   without usage: line (by setting argv[0] to the empty
	   string) */
	if (options.show_help) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0][0] = '\0';
	}

	ret = fuse_main(args.argc, args.argv, &hello_oper, NULL);
	fuse_opt_free_args(&args);
	return ret;
}
