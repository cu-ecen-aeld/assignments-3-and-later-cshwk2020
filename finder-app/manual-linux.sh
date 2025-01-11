#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

FINDER_APP_DIR=$(pwd)
OUTDIR=/mnt/sda4/tmp/aeld
KERNEL_DL_DIR=/mnt/sda4/linux_kernel/linux-5.1.10
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
#KERNEL_VERSION=v5.15.163
KERNEL_VERSION=v5.1.10
#BUSYBOX_DL_DIR=/mnt/sda4/linux_kernel/buildroot-2021.02.12
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
#CROSS_COMPILE=aarch64-none-linux-gnu-
CROSS_COMPILE=aarch64-none-linux-gnu-
TOOLCHAIN_BASE=/mnt/sda4/bin/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu
TOOLCHAIN_LIBC_DIR="${TOOLCHAIN_BASE}/aarch64-none-linux-gnu/libc"


if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}


cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	#echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	#git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
    mkdir -p ${OUTDIR}/linux-stable
    cp -ru ${KERNEL_DL_DIR}/* ${OUTDIR}/linux-stable
    
    # TODO: Add your kernel build steps here
    cd linux-stable
    make ARCH=arm64 defconfig
    make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- 

fi

#if [ -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
#    cd linux-stable
#    echo "Checking out version ${KERNEL_VERSION}"
#    #git checkout ${KERNEL_VERSION}
#
#fi


echo "Adding the Image in outdir"
cp "${OUTDIR}/linux-stable/arch/arm64/boot/Image" ${OUTDIR} 


cd "$OUTDIR"

# if [ -d "${OUTDIR}/rootfs" ]
# then
# 	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
#     sudo rm  -rf ${OUTDIR}/rootfs
# fi

# TODO: Create necessary base directories
if [ ! -d "${OUTDIR}/rootfs" ]
then
    echo "Creating the staging directory for the root filesystem"
    
    cd "$OUTDIR"
    mkdir -p ./rootfs
    cd rootfs
    mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var 
    mkdir -p usr/bin usr/lib usr/sbin
    mkdir -p var/log
fi 

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
    git clone git://busybox.net/busybox.git
    #cp -r "${BUSYBOX_DL_DIR}/*" "${OUTDIR}/busybox/"
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- defconfig

    # # TODO: Make and install busybox
    make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- 
    make CONFIG_PREFIX="${OUTDIR}/rootfs" ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- install

else
    cd busybox
fi



# TODO: Add library dependencies to rootfs
# lib - program interpreter,  
# lib64 - shared lib

echo "Library dependencies"
cd ${OUTDIR}/rootfs
pi_text=$(${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter")
shared_lib_text=$(${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library")
echo $shared_lib_text


echo "coping program interpreter lib"
echo $pi_text
pi_relpath=$(echo $pi_text | sed 's/.*: \(.*\)]/\1/' )
echo $pi_relpath
#
cp  "${TOOLCHAIN_LIBC_DIR}${pi_relpath}" "${OUTDIR}/rootfs/lib"


echo "coping shared lib64"
shared_lib_matches=$(echo $shared_lib_text | grep -oP '\[\K[^\]]+(?=\])')
for shared_lib_item in $shared_lib_matches; do
    echo $shared_lib_item
    #
    cp "${TOOLCHAIN_LIBC_DIR}/lib64/${shared_lib_item}" "${OUTDIR}/rootfs/lib64"
done



# TODO: Make device nodes
cd ${OUTDIR}/rootfs
if [ ! -e dev/null ]; then 
    mknod -m 666 dev/null c 1 3
fi 
if [ ! -e dev/tty ]; then 
    mknod -m 666 dev/tty c 5 1
fi 
if [ ! -e dev/console ]; then 
    mknod -m 666 dev/console c 5 1
fi 

# TODO: Clean and build the writer utility
# CROSS CMPILE writer.c
make CROSS_COMPILE=aarch64-none-linux-gnu- TARGET="${OUTDIR}/rootfs/home/writer" -C "${FINDER_APP_DIR}" 


# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
# home <- finder.sh, -rr conf, finder-test.sh
# finder-test.sh: modify conf/XXX instead of ../conf/XXX
cp "${FINDER_APP_DIR}/autorun-qemu.sh" "${OUTDIR}/rootfs/home"
cp "${FINDER_APP_DIR}/finder.sh" "${OUTDIR}/rootfs/home"
cp "${FINDER_APP_DIR}/finder-test.sh" "${OUTDIR}/rootfs/home"
mkdir -p "${OUTDIR}/rootfs/home/conf"
cp ${FINDER_APP_DIR}/../conf/* "${OUTDIR}/rootfs/home/conf"
# modify finder-test.sh remove ../ from config/xxx
sed -i 's|\.\./conf/assignment|conf/assignment|g' "${OUTDIR}/rootfs/home/finder-test.sh"

# TODO: Chown the root directory
# TODO: Create initramfs.cpio.gz
cd ${OUTDIR}/rootfs
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio.gz
