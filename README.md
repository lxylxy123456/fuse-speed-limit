# fuse-speed-limit
A FUSE file system that limits speed

## Usage

Build and mount to `mnt`.
```sh
cmake -B build --preset develop
cmake --build build
mkdir mnt
./build/hello/hello mnt
```

Write and read file with speed limit 10 KiB/s. (`mnt/0` has no speed limit.)
```sh
dd if=/dev/zero of=mnt/10240/a.txt bs=1K count=30
dd if=mnt/10240/a.txt of=/dev/null bs=1K count=30
```

Get the last read and write location using ioctl
```sh
dd if=/dev/zero of=mnt/10240/a.txt bs=1K seek=10 count=1 conv=notrunc
dd if=mnt/10240/a.txt of=/dev/null bs=1K skip=15 count=1 conv=notrunc
python3 hello/hello_ioctl.py mnt/10240/a.txt    # 16384 11264
```

## Related

I wrote this project to report this Firefox feature request:
* <https://connect.mozilla.org/t5/ideas/unable-to-cancel-download-when-firefox-is-moving-part-file/idi-p/139448>
* <https://youtu.be/vn0RJVk5JS4>

