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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <dynamic_list.h>

struct item {
	int id;
	 DL_ITEM_STRUCT(p, struct item);
};

struct head {
	DL_HEAD_STRUCT(l, struct item);
} h;

int main()
{
	struct item *a = malloc(sizeof(struct item));
	assert(a);
	a->id = 12;
	dl_append(&h.l, a, p);

	struct item *b = malloc(sizeof(struct item));
	assert(b);
	b->id = 14;
	dl_append(&h.l, b, p);

	struct item *c = malloc(sizeof(struct item));
	assert(c);
	c->id = 16;
	dl_append(&h.l, c, p);

	dl_remove(&h.l, b, p);

	dl_for(&h.l, i, p) {
		printf("%d ", i->id);
	}
	printf("\n");

	return 0;
}
