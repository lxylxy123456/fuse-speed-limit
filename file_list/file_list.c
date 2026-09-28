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

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#include <file_list.h>

struct fl_head {
	DL_HEAD_STRUCT(fl, struct fl_item);
} fl_head = {.fl = DL_HEAD_INITIALIZER };

pthread_mutex_t file_list_mutex = PTHREAD_MUTEX_INITIALIZER;

void fl_lock(void)
{
	assert(pthread_mutex_lock(&file_list_mutex) == 0);
}

void fl_unlock(void)
{
	assert(pthread_mutex_unlock(&file_list_mutex) == 0);
}

void fli_lock(fl_item_t *fli)
{
	assert(pthread_mutex_lock(&fli->lock) == 0);
}

void fli_unlock(fl_item_t *fli)
{
	assert(pthread_mutex_unlock(&fli->lock) == 0);
}

int fl_compute_path(fl_item_t *fli, const char *path)
{
	fli->path = strdup(path);
	assert(fli->path);
	fli->dir = strdup(path);
	assert(fli->dir);
	fli->name = strchr(fli->dir + 1, '/');
	assert(fli->name);
	fli->name[0] = '\0';
	fli->name++;
	return 0;
}

void fl_free_path(fl_item_t *fli)
{
	free(fli->dir);
	free(fli->path);
}

fl_item_t *fl_new(const char *path)
{
	fl_item_t *fli = malloc(sizeof(fl_item_t));
	assert(fli);

	dl_append(&fl_head.fl, fli, fl);
	assert(fl_compute_path(fli, path) == 0);
	pthread_mutex_init(&fli->lock, NULL);
	fli->refcount = 1;
	fli->size = 0;
	fli->speed = 0;
	assert(clock_gettime(CLOCK_REALTIME, &fli->atime) == 0);
	fli->mtime = fli->atime;
	fli->ctime = fli->atime;
	fli->read_loc = 0;
	fli->write_loc = 0;

	return fli;
}

void fl_free(fl_item_t *fli)
{
	fli_lock(fli);
	assert(fli->refcount == 1);
	fli_unlock(fli);

	dl_remove(&fl_head.fl, fli, fl);

	fl_free_path(fli);
	free(fli);
}

fl_item_t *fl_first(void)
{
	return dl_first(&fl_head.fl);
}

fl_item_t *fl_next(fl_item_t *cur)
{
	return dl_next(cur, fl);
}
