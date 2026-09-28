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

#ifndef _HELLO_IOCTL_H
#define _HELLO_IOCTL_H

#include <sys/ioctl.h>

typedef struct {
	size_t read_loc;
	size_t write_loc;
} hello_ioctl_getinfo_t;

#define HELLO_IOCTL_GETINFO _IOR('Y', 0, hello_ioctl_getinfo_t)

// If static asserts fail, change here and Python code.
_Static_assert(HELLO_IOCTL_GETINFO == 0x80105900, "Constant changed");

#endif				/* _HELLO_IOCTL_H */
