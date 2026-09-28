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

#ifndef _FILE_LIST_H
#define _FILE_LIST_H

#include <dynamic_list.h>

typedef struct fl_item {
	DL_ITEM_STRUCT(fl, struct fl_item);
	char *path;
	char *dir;
	char *name;
	// For members below, use fli_lock(&lock).
	// For path, use both fl_lock() and fli_lock(&lock).
	// For fl, use fl_lock().
	// Lock order: fl_lock() then fli_lock(&lock);
	pthread_mutex_t lock;
	size_t refcount;
	size_t size;
	size_t speed;
	struct timespec atime;
	struct timespec mtime;
	struct timespec ctime;
	size_t read_loc;
	size_t write_loc;
} fl_item_t;

void fl_lock(void);
void fl_unlock(void);
void fli_lock(fl_item_t * fli);
void fli_unlock(fl_item_t * fli);

int fl_compute_path(fl_item_t * fli, const char *path);
void fl_free_path(fl_item_t * fli);

fl_item_t *fl_new(const char *path);
void fl_free(fl_item_t * fli);
fl_item_t *fl_first(void);
fl_item_t *fl_next(fl_item_t * cur);

#endif				/* _FILE_LIST_H */
