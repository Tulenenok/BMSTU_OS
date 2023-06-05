## Лабораторная работа №5

Виртуальная файловая система

Есть такой список команд, в целом он отвечает на все вопросы:

```bash
sudo insmod myfs.ko
sudo mount -t myfs none /home/parallels/Desktop/labVFS/test2/
sudo mount -t myfs none /home/parallels/Desktop/labVFS/test1/
sudo dmesg
lsmod
sudo cat /proc/mounts
sudo cat /proc/filesystems
sudo cat /proc/slabinfo
stat -f -c=%t /home/parallels/Desktop/labVFS/test1
sudo umount /home/parallels/Desktop/labVFS/test2
sudo umount /home/parallels/Desktop/labVFS/test1
sudo cat /proc/mounts
sudo cat /proc/slabinfo
lsmod
sudo rmmod myfs.ko
sudo cat /proc/filesystems
```

* буфер какой? 

  кольцевой, буфер пользователя.

* Почему буфер пользователя может быть выгружен?

   ядро работает с физическими адресами, а процесс имеет виртуальное адресное пространство, и проблема в том, что страница, в которой находится буфер пользователя, может быть выгружена. Может быть выгружена в область свопинга из-за того, что произойдет вытеснение, поэтому надо использовать функции copy to user и copy from user.
