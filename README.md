# nxtrc Vers 2.3
Copyright 2010 by GiP

> This program is distributed under the terms of the
> GNU General Public License (GPL)
> See file COPYING
> ChangeLog at the end

This is the new version of _nxtrc_ (NXT Remote Command) a program to interact
with NXT using BlueTooth on Linux. This was written and tested on a Linux
Fedora Core 4 but it works on the latest Fedora (12+). It has been reported
to work on Ubuntu 6.10 and Suse 10.2 and Ubuntu 20.04 too, and I expect it will
work on most modern distributions.

It uses a group of functions that can be linked in programs that want to
interact with NXT through BT. It's heavily commented so you can use it also as
a hint on how to write your own functions.

It's a small program so I didn't make a library out of the functions. Feel
free to modify it for your use.

## Install

Required prerequisite is the Bluez development package (`libbluetooth-dev` on
Debian/Ubuntu).

To use it, run `./configure` and `make`

|File           | Description                 |
|---------------|-----------------------------|
|`nxtrc.c`      | Source of the program       |
|`nxtrc_func.c` | Functions used in `nxtrc.c` |
|`nxtrc_func.h` | Header with definitions, use it as a User Guide for the functions.|
|`nxtrc.1`      | man page for nxtrc          |
|`README.md`    | this file                   |

To use nxtrc first run `nxtrc -s` to scan for BT devices then either run

```
nxtrc -a XX:XX:XX:XX:XX:XX <command>
```

or put the address in an Environment Variable `BTADDR` with (bash)

```
export BTADDR="XX:XX:XX:XX:XX:XX"
```

and then run

```
nxtrc <command>
```

Remember that, as with almost all Bluetooth devices, the first time you use it
with a NXT you'll have to exchange a password. How this will be done depends
by the Operative System and Environment you use......

Running nxtrc without options or with `-h` will give the usage help like this:

```
  Usage: nxtrc [-s | -a ADDR command] [-v]
        -a  ADDR     Connect with NXT at ADDR
                       required unless -s is used
                       or ADDR is in BTADDR Env Var
        -v           Be verbose
      and one command
        -i           Get info on NXT brick
        -l[PATT]     List files on NXT brick
                       matching PATT if present (e.g. -l*.rxe)
                       N.B. No spaces between -l and PATT!
        -b           Get Battery Level
        -n  NAME     Set NXT name to NAME
        -w  FILE     Write FILE on NXT
        -W  FILE     Same as -w overwriting FILE if exists
        -r  FILE     Read FILE frome NXT and save it
        -d  FILE     Delete FILE frome NXT
        -p  FILE     Run Program FILE on NXT
        -k           Stop a program running on NXT
        -m  FILE     Play Sound File once
        -M  FILE     Play Sound File indefinitely
        -z           Stop Sound Playback

      or  -s         Scan for BT devices

    if more than one command is given only
    the last one is performed.
```

## Extra Stuff

You'll find also a weird Example of the use of nxtrc functions and BT on
Linux:

(To use this you'll need John Hansen's NBC/NXC from
 http://bricxcc.sourceforge.net/nbc/)

Not the usual kind of example but it's something that we plan to do later
so I started with this kind of stuff and I might as well put it in, someone
might find it useful.....

---
A program runs on the NXT (config with A and C motors and 1 Light Sensor
in fronti pointing down). The NXT on a table moves and tries to avoid the
border looking for the reflection on the surface,  backs up, turns and
restarts (not always be careful!) and sends data to the PC (Chann 0,
master) data is time, sensor, and Tacho of both motors.

A program runs on the PC. It starts the program on the NXT and collects
and outputs the data.

|File          | Description            |
|--------------|------------------------|
|`SendDat.nxc` | NXC prog for the brick |
|`GetData.c`   | C prog for the PC      |

To make it work :

1. Compile `GetData` with `gcc`
  ```
  gcc -o GetData GetData.c nxtrc_func.c -lbluetooth -lm
  ```
2. Compile `SendDat.rxe` with `nbc`
  ```
  nbc -O=SendDat.rxe SendDat.nxc
  ```
3. Use `nxtrc` to download `SendDat.rxe` on the Brick
  (add `-a BTADDRESS` or put it in the Environment variable `BTADDR`)
  ```
  nxtrc -W SendDat.rxe
  ```
4. Start `GetData` [redirecting output to a file]
  ```
  GetData  [>Data.dat]
  ```
5. plot `Data.dat` (with xmgrace or whatever you want)
  ```
  xmgrace -nxy Data.dat
  ```

That's all,
GiP

P.S.: If you want to contact me you can E-Mail to
<gianpiero.puccioni@isc.cnr.it>.
Please put NXT or NXTRC somewhere in the subject.
Or open an issue in this GitGub repository.

-----------------------------------------------------
## CHANGELOG
### Version 2.3
- changed the name to lowercase nxtrc as an all uppercase program is
  nonstandard.
- Thanks to Damian Wrobel now `nxtrc` comes with proper configure and
  Makefile and it looks as it was made by someone who knows what he is
  doing (i.e. not me).
- fixed a bug in the `ReadFile` function (files were sometimes written with
  fewer bytes)
- fixed bug that reported Firmware 1.03 as 1.3
- added a function `FileExist` that check if a file exists on the NXT. The
  function is used before trying to run a program or a sound file as the
  NXT gave weird responses if it doesn't.
- other small fixes.

### Version 2.2
- Changed delay in reading file as long files sometimes didn't
    transfer.
- Added function `GetFree`(s) to get free space for files on the
    device
- Added Free Space to the file listing output
- changed a "`char`" to "`unsigned char`" as sometimes it gave the
   wrong file size when writing.
- more small bugfixes (and bigger ones...)

### Version 2.1
- small bugfixes

### Version 2.0
- No added functionality
- Some comments rewritten for clarity
- Complete rewrite of functions to allow connections to multiple
  NXT. All (almost) functions have now as first parameter `<s>`,
  the socket returned by `nxt_connect`.
- This break compatibility with older versions, hence the 2.0  

### Version 1.2
- Option `-L` eliminated: now `-l` accepts optional parameter (but it must
  follow it with no spaces in between  

### Version 1.1
- bugs fixed

### Version 1.0
- first version - some bugs  
