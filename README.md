BigFileCopy
===========

copy a big file picewice by creating signature files

Algorithm
=========

The input file is partitioned into chunks (at the moment of fix size). Every chunk is then used to calculate the checksum.

All checksums are stored in a file with the same filename as the input file with an extension .hc (hash codes).

Later this checksumfile (at the target disk) is used to find out those parts that have changed since the last snapshot

Creating snapshots
=======

To create a snapshot of a potentially big file onto a remote file system that does not support the rsync algorithm (e.g. an usb drive) 
use this command line

    bfcp snapshot srcFile trgtFile
  
This command will generate a hc-file for the source file and for the first time both files (source and hc file) are copied to the destination.

Later when the source file have portions that have changed the same command line is used to update the (remote) snapshot.

Only the changed parts are transferted. First the local hashfile is updated and if there are changes to the target hash file these parts 
are updated at the target.

Creating hashfiles only
======

If you only want to recalculate the hashfile without transfering it to the source filesystem this command line can be used

    bfcp hash file
