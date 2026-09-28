#	fuse-speed-limit - A FUSE file system that limits speed
#	Copyright (C) 2026  lxylxy123456
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.31)

project(lxylxy123456_fuse)

add_subdirectory(dynamic_list)
add_subdirectory(dynamic_list_example)
add_subdirectory(file_list)
add_subdirectory(cmake_fuse)
add_subdirectory(hello)

