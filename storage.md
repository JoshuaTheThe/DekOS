instead of root being a mirror of the boot drive + a directory of devices, the goal is to instead have root be just the latter,<br>
containing information about what drive is in use, or in other words a path may look like:<br>

/tty0/stdout<br>
or<br>
/ide0/system/system.ini<br>

of course, the device can be omitted in context,<br>
for instance the drive that the program was launch on<br>
