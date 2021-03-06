/*
 * Copyright (c) 2011 Novell, Inc.
 *
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, contact Novell, Inc.
 *
 * To contact Novell about this file by physical or electronic mail, you may
 * find current contact information at www.novell.com.
 */


#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <mntent.h>

#include "snapper/Log.h"
#include "snapper/Filesystem.h"
#include "snapper/Snapper.h"
#include "snapper/SnapperTmpl.h"
#include "snapper/SystemCmd.h"
#include "snapper/SnapperDefines.h"


namespace snapper
{

    Filesystem*
    Filesystem::create(const string& fstype, const string& subvolume)
    {
	if (fstype == "btrfs")
	    return new Btrfs(subvolume);

	if (fstype == "ext4")
	    return new Ext4(subvolume);

	throw InvalidConfigException();
    }


    Btrfs::Btrfs(const string& subvolume)
	: Filesystem(subvolume)
    {
	if (access(BTRFSBIN, X_OK) != 0)
	{
	    throw ProgramNotInstalledException(BTRFSBIN " not installed");
	}
    }


    void
    Btrfs::createConfig() const
    {
	SystemCmd cmd2(BTRFSBIN " subvolume create " + quote(subvolume + "/.snapshots"));
	if (cmd2.retcode() != 0)
	    throw CreateConfigFailedException("creating btrfs snapshot failed");
    }


    void
    Btrfs::deleteConfig() const
    {
	SystemCmd cmd2(BTRFSBIN " subvolume delete " + quote(subvolume + "/.snapshots"));
	if (cmd2.retcode() != 0)
	    throw DeleteConfigFailedException("deleting btrfs snapshot failed");
    }


    string
    Btrfs::infosDir() const
    {
	return (subvolume == "/" ? "" : subvolume) + "/.snapshots";
    }


    string
    Btrfs::snapshotDir(unsigned int num) const
    {
	return (subvolume == "/" ? "" : subvolume) + "/.snapshots/" +
	    decString(num) + "/snapshot";
    }


    void
    Btrfs::createSnapshot(unsigned int num) const
    {
	SystemCmd cmd(BTRFSBIN " subvolume snapshot -r " + quote(subvolume) + " " +
		      quote(snapshotDir(num)));
	if (cmd.retcode() != 0)
	    throw CreateSnapshotFailedException();
    }


    void
    Btrfs::deleteSnapshot(unsigned int num) const
    {
	SystemCmd cmd(BTRFSBIN " subvolume delete " + quote(snapshotDir(num)));
	if (cmd.retcode() != 0)
	    throw DeleteSnapshotFailedException();
    }


    bool
    Btrfs::isSnapshotMounted(unsigned int num) const
    {
	return true;
    }


    void
    Btrfs::mountSnapshot(unsigned int num) const
    {
    }


    void
    Btrfs::umountSnapshot(unsigned int num) const
    {
    }


    bool
    Btrfs::checkSnapshot(unsigned int num) const
    {
	return checkDir(snapshotDir(num));
    }


    Ext4::Ext4(const string& subvolume)
	: Filesystem(subvolume)
    {
	if (access(CHSNAPBIN, X_OK) != 0)
	{
	    throw ProgramNotInstalledException(CHSNAPBIN " not installed");
	}

	if (access(CHATTRBIN, X_OK) != 0)
	{
	    throw ProgramNotInstalledException(CHATTRBIN " not installed");
	}
    }


    void
    Ext4::createConfig() const
    {
	int r1 = mkdir((subvolume + "/.snapshots").c_str(), 700);
	if (r1 == 0)
	{
	    SystemCmd cmd1(CHATTRBIN " +x " + quote(subvolume + "/.snapshots"));
	    if (cmd1.retcode() != 0)
		throw CreateConfigFailedException("chattr failed");
	}
	else if (errno != EEXIST)
	{
	    y2err("mkdir failed errno:" << errno << " (" << strerror(errno) << ")");
	    throw CreateConfigFailedException("mkdir failed");
	}

	int r2 = mkdir((subvolume + "/.snapshots/.info").c_str(), 700);
	if (r2 == 0)
	{
	    SystemCmd cmd2(CHATTRBIN " -x " + quote(subvolume + "/.snapshots/.info"));
	    if (cmd2.retcode() != 0)
		throw CreateConfigFailedException("chattr failed");
	}
	else if (errno != EEXIST)
	{
	    y2err("mkdir failed errno:" << errno << " (" << strerror(errno) << ")");
	    throw CreateConfigFailedException("mkdir failed");
	}
    }


    void
    Ext4::deleteConfig() const
    {
	int r1 = rmdir((subvolume + "/.snapshots/.info").c_str());
	if (r1 != 0)
	{
	    y2err("rmdir failed errno:" << errno << " (" << strerror(errno) << ")");
	    throw DeleteConfigFailedException("rmdir failed");
	}

	int r2 = rmdir((subvolume + "/.snapshots").c_str());
	if (r2 != 0)
	{
	    y2err("rmdir failed errno:" << errno << " (" << strerror(errno) << ")");
	    throw DeleteConfigFailedException("rmdir failed");
	}
    }


    string
    Ext4::infosDir() const
    {
	return (subvolume == "/" ? "" : subvolume) + "/.snapshots/.info";
    }


    string
    Ext4::snapshotDir(unsigned int num) const
    {
	return subvolume + "@" + decString(num);
    }


    string
    Ext4::snapshotFile(unsigned int num) const
    {
	return (subvolume == "/" ? "" : subvolume) + "/.snapshots/" + decString(num);
    }


    void
    Ext4::createSnapshot(unsigned int num) const
    {
	SystemCmd cmd1(TOUCHBIN " " + quote(snapshotFile(num)));
	if (cmd1.retcode() != 0)
	    throw CreateSnapshotFailedException();

	SystemCmd cmd2(CHSNAPBIN " +S " + quote(snapshotFile(num)));
	if (cmd2.retcode() != 0)
	    throw CreateSnapshotFailedException();
    }


    void
    Ext4::deleteSnapshot(unsigned int num) const
    {
	SystemCmd cmd(CHSNAPBIN " -S " + quote(snapshotFile(num)));
	if (cmd.retcode() != 0)
	    throw DeleteSnapshotFailedException();
    }


    bool
    Ext4::isSnapshotMounted(unsigned int num) const
    {
	FILE* f = setmntent("/etc/mtab", "r");
	if (!f)
	{
	    y2err("setmntent failed");
	    throw IsSnapshotMountedFailedException();
	}

	bool mounted = false;

	struct mntent* m;
	while ((m = getmntent(f)))
	{
	    if (strcmp(m->mnt_type, "rootfs") == 0)
		continue;

	    if (m->mnt_dir == snapshotDir(num))
	    {
		mounted = true;
		break;
	    }
	}

	endmntent(f);

	return mounted;
    }


    void
    Ext4::mountSnapshot(unsigned int num) const
    {
	if (isSnapshotMounted(num))
	    return;

	SystemCmd cmd1(CHSNAPBIN " +n " + quote(snapshotFile(num)));
	if (cmd1.retcode() != 0)
	    throw MountSnapshotFailedException();

	int r1 = mkdir(snapshotDir(num).c_str(), 0755);
	if (r1 != 0 && errno != EEXIST)
	{
	    y2err("mkdir failed errno:" << errno << " (" << strerror(errno) << ")");
	    throw MountSnapshotFailedException();
	}

	SystemCmd cmd2(MOUNTBIN " -t ext4 -r -o loop,noload " + quote(snapshotFile(num)) +
		       " " + quote(snapshotDir(num)));
	if (cmd2.retcode() != 0)
	    throw MountSnapshotFailedException();
    }


    void
    Ext4::umountSnapshot(unsigned int num) const
    {
	if (!isSnapshotMounted(num))
	    return;

	SystemCmd cmd1(UMOUNTBIN " " + quote(snapshotDir(num)));
	if (cmd1.retcode() != 0)
	    throw UmountSnapshotFailedException();

	SystemCmd cmd2(CHSNAPBIN " -n " + quote(snapshotFile(num)));
	if (cmd2.retcode() != 0)
	    throw UmountSnapshotFailedException();

	rmdir(snapshotDir(num).c_str());
    }


    bool
    Ext4::checkSnapshot(unsigned int num) const
    {
	return checkNormalFile(snapshotFile(num));
    }

}
