#initialize 480MiB to 0
sudo dd if=/dev/zero of=/dev/sdb bs=4k count=120000
sync
# First sector: msdos
sudo parted /dev/sdb mklabel msdos
#copy sunxi-spl.bin binaries
sudo dd if=/buildroot/output/images/sunxi-spl.bin of=/dev/sdb bs=512 seek=16
#copy u-boot
sudo dd if=/buildroot/output/images/u-boot.itb of=/dev/sdb bs=512 seek=80
# 1st partition: 64MiB: (163840-32768)*512/1024 = 64MiB
sudo parted /dev/sdb mkpart primary fat32 32768s 163839s
# 2nd partition: 1GiB: 4358144-163840)*512/1024 = 2GiB
sudo parted /dev/sdb mkpart primary ext4 163840s 4358143s
sudo mkfs.vfat /dev/sdb1
sudo mkfs.ext4 /dev/sdb2 -L rootfs
sync
#copy kernel, flattened device tree, boot.scr
sudo mount /dev/sdb1 /run/mount
sudo cp /buildroot/output/images/Image /run/mount
sudo cp /buildroot/output/images/nanopi-neo-plus2.dtb /run/mount
sudo cp /buildroot/output/images/boot.scr /run/mount
sync

#Rename 1st partition to BOOT
sudo umount /dev/sdb1
sudo fatlabel /dev/sdb1 BOOT
#copy rootfs
sudo dd if=/buildroot/output/images/rootfs.ext4 of=/dev/sdb2
# Resize and rename 2nd partition to rootfs
# check if the partition must be mounted
sudo e2fsck -f /dev/sdb2
sudo resize2fs /dev/sdb2
sudo e2label /dev/sdb2 rootfs