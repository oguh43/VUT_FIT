#!/bin/bash

# Malo by byt podla zadania
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

# 1) Vytvor 200 MB subor /var/tmp/ukol.img
echo "1) Creating 200 MB file /var/tmp/ukol.img"
dd if=/dev/zero of=/var/tmp/ukol.img bs=1M count=200

# 2)Vytvor loop device pre ukol.img
echo "2) Creating loop device for /var/tmp/ukol.img"
losetup -fP /var/tmp/ukol.img
LOOP_DEV=$(losetup -j /var/tmp/ukol.img | cut -d':' -f1)

# 3) Vytvor ext4 filesystem len na novom loop device
echo "3) Creating ext4 filesystem on $LOOP_DEV"
mkfs.ext4 $LOOP_DEV

# 4) Uprav /etc/fstab aby automaticky mountol fs cez ld
echo "4) Modifying /etc/fstab to mount $LOOP_DEV at /var/www/html/ukol"
mkdir -p /var/www/html/ukol
echo "$LOOP_DEV /var/www/html/ukol ext4 defaults 0 0" >> /etc/fstab

# 5) Mountni fs na /var/www/html/ukol
echo "5) Mounting filesystem at /var/www/html/ukol"
mount $LOOP_DEV /var/www/html/ukol

# 6) Stiahni baliky z args do /var/www/html/ukol
echo "6) Downloading packages: $@"
yum install --downloadonly --downloaddir=/var/www/html/ukol "$@"

# 7) Vygeneruj repodata in /var/www/html/ukol a obnov SELinux context
echo "7) Generating repodata in /var/www/html/ukol and restoring SELinux context"
yum install -y createrepo
createrepo /var/www/html/ukol
restorecon -Rv /var/www/html/ukol

# 8) Nastav /etc/yum.repos.d/ukol.repo aby pristupoval do repa "ukol" cez http://localhost/ukol
echo "8) Configuring /etc/yum.repos.d/ukol.repo to access http://localhost/ukol."
echo "[ukol]" > /etc/yum.repos.d/ukol.repo
echo "name=Local Ukol Repository" >> /etc/yum.repos.d/ukol.repo
echo "baseurl=http://localhost/ukol" >> /etc/yum.repos.d/ukol.repo
echo "enabled=1" >> /etc/yum.repos.d/ukol.repo
echo "gpgcheck=0" >> /etc/yum.repos.d/ukol.repo

# 9) Nainstaluj a spusti Apachee server (httpd)
echo "9) Installing and starting Apache web server"
yum install -y httpd
systemctl enable --now httpd

# 10) Vypis yum repa z "ukol"
echo "10) Verifying that the 'ukol' repository is available"
yum repolist

# 11) Unmountni fs /var/www/html/ukol
echo "11) Unmounting filesystem from /var/www/html/ukol"
umount /var/www/html/ukol

# 12) Spusti "mount -a" a over ze sa fs mountlo (pripojilo)
echo "12) Running 'mount -a' to remount the filesystem"
mount -a
if mountpoint -q /var/www/html/ukol; then
    echo "/var/www/html/ukol is successfully mounted"
else
    echo "Failed to mount /var/www/html/ukol"
fi

# 13) Ukaz baliky z repa "ukol"
echo "13) Displaying package info from 'ukol' repository"
yum --disablerepo="*" --enablerepo="ukol" info