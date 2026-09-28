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

#ifndef _DYNAMIC_LIST_H
#define _DYNAMIC_LIST_H

#define DL_ITEM_STRUCT(name, type) \
	struct { \
		type *prev; \
		type *next; \
	} name

#define DL_HEAD_STRUCT(name, type) \
	struct { \
		type *first; \
		type *last; \
	} name

#define DL_ITEM_INITIALIZER { .prev=NULL, .next=NULL }
#define DL_HEAD_INITIALIZER { .first=NULL, .last=NULL }

#define dl_first(head) \
	((head)->first)

#define dl_last(head) \
	((head)->last)

#define dl_next(item, path) \
	((item)->path.next)

#define dl_prev(item, path) \
	((item)->path.prev)

#define dl_append_left(head, item, path) \
	do { \
		if ((head)->last == NULL) { \
			(head)->last = (item); \
			(item)->path.next = NULL; \
		} else { \
			(item)->path.next = (head)->first; \
			(head)->first->path.prev = (item); \
		} \
		(item)->path.prev = NULL; \
		(head)->first = (item); \
	} while(0)

#define dl_append(head, item, path) \
	do { \
		if ((head)->first == NULL) { \
			(head)->first = (item); \
			(item)->path.prev = NULL; \
		} else { \
			(item)->path.prev = (head)->last; \
			(head)->last->path.next = (item); \
		} \
		(item)->path.next = NULL; \
		(head)->last = (item); \
	} while(0)

#define dl_remove(head, item, path) \
	do { \
		if ((item)->path.prev == NULL) { \
			(head)->first = (item)->path.next; \
		} else { \
			(item)->path.prev->path.next = (item)->path.next; \
		} \
		if ((item)->path.next == NULL) { \
			(head)->last = (item)->path.prev; \
		} else { \
			(item)->path.next->path.prev = (item)->path.prev; \
		} \
		(item)->path.next = NULL; \
		(item)->path.prev = NULL; \
	} while(0)

#define dl_for(head, iter, path) \
	for (typeof((head)->first) iter = (head)->first; \
	     iter; \
	     iter = iter->path.next)

#endif				/* _DYNAMIC_LIST_H */
