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

import argparse, time, fcntl, struct

HELLO_IOCTL_GETINFO = 0x80105900
hello_ioctl_getinfo_t = struct.Struct('LL')

def run_ioctl(f):
	arg = bytearray(hello_ioctl_getinfo_t.size)
	fcntl.ioctl(f, HELLO_IOCTL_GETINFO, arg)
	rloc, wloc = hello_ioctl_getinfo_t.unpack(arg)
	return rloc, wloc

def main():
	parser = argparse.ArgumentParser()
	parser.add_argument('file')
	parser.add_argument('-w', '--watch', action='store_true')
	args = parser.parse_args()

	with open(args.file, 'r') as f:
		while True:
			rloc, wloc = run_ioctl(f)
			print(rloc, wloc)
			if args.watch:
				time.sleep(1)
				continue
			break

if __name__ == '__main__':
	main()

