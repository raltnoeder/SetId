# SetId
**Run commands under an arbitrary user id, group id and supplemental group list**

## Usage
**Syntax:**  
`setid [ userid=<uid_nr> ] [ groupid=<gid_nr> ] [ groups=<gid_nr>,<gid_nr>,... ] command <executable_path> <arguments...>`

The `setid` enables running the command following the `command` keyword under an arbitrary user id, group id and/or supplemental group list.

The value for the `userid`, `groupid` and `groups` parameters must be a numeric id.  
(The utility does not support resolution from user or group names to their numeric ids)

Obviously, for the `setid` utility to work, it has to be run under an adequately privileged user id.
No less obviously is it a **security vulnerability** to enable such privileges on the `setid` program file when it is started by untrusted users (in other words, don't make this program setuid-root and don't set privileges on it that allow untrusted users to manipulate their own ids).

**Usage example:**
As `root`:
`setid userid=1770 groupid=600 groups=4001,4002,4035 command /bin/ls -R /home/public/Documents`
Run the command `/bin/ls -R /home/public/Documents` under the user id 1770, group id 600, and with the supplemental group ids 4001, 4002 and 4035

`setid groupid=770 command /bin/zsh`
Run the `/bin/zsh` shell under group id 770, no supplemental groups, leave the user id unchanged

## Supported systems
The `setid` utility works on most UNIX-like operating systems.

**Standards**  
SVr4, 4.3BSD

**Supported operating systems**  
IBM AIX  
Oracle Solaris / OpenSolaris / Illumos (SunOS 5.x)  
FreeBSD  
OpenBSD  
NetBSD  
Linux  
Hewlett-Packard HP/UX  
Hewlett-Packard Tru64 UNIX  
SGI *(now Hewlett-Packard Enterprise)* IRIX  
Apple Mac OS X  
z/OS *(provided setgroups() -> BPX1SGR/BPX4SGR is available)*  

## Compiling

Requires the [cppdsaext](https://github.com/raltnoeder/cppdsaext) library.

```
cd cppdsaext/src
make -j 8 all
cd ../..
cd setid/src
c++ -std=c++11 -Wall -Werror -I . -I ../../cppdsaext/src -c GroupIdList.cpp
c++ -std=c++11 -Wall -Werror -I . -I ../../cppdsaext/src -c SetId.cpp
c++ -o setid SetId.o GroupIdList.o ../../cppdsaext/src/dsaext.o ../../cppdsaext/src/integerparse.o
```

You may want to add additional compiler-specific options, such as `-static-libstdc++` and `-static-libgcc` when using the GCC compiler to generate a statically linked binary, or other options to link against shared libraries.
