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

add_executable(hello)

target_sources(hello
  PRIVATE
    hello.c

  PRIVATE
    FILE_SET HEADERS
    FILES
      hello_ioctl.h
)

target_link_libraries(hello
  PRIVATE
    file_list
    cmake_fuse
)

option(HELLO_FAST "..." 0)
target_compile_definitions(hello PRIVATE "HELLO_FAST=${HELLO_FAST}")

