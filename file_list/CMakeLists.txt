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

add_library(file_list)

target_sources(file_list
  PRIVATE
    file_list.c

  PUBLIC
    FILE_SET HEADERS
    FILES
      file_list.h
)

target_link_libraries(file_list
  PUBLIC
    dynamic_list
)

