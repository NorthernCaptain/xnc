        Welcome to X Northern Captain 4.4.x !!!
                 (http://xnc.dubna.su)

-----------------
Introduction:
-----------------
        
X Northern Captain (XNC) - filemanager for X Window.
XNC - uses conception of two panels. These panels with highlighted directory
listings give You simple and easy way to manipulate files on Your system.
Bookmark feature allow You to remember Hot Directories and quick changing
to it. Virtual File System can manipulate files in popular archieve formats as easy
as files on normal Disk FS (copy/delete/view/edit/execute inside archieve). 
MultiWindow Editing and Viewing System (IVES) edit text files and view ASCII/HEX 
and many image formats ; built in xterminal (rxvt 2.20) shows output from applications;
configurable User menu do quick execute for usefull commands;
exention association and execution list allow execute application by associated extension;
processes information shows list of processes and can send signal to its; disk and
memory summary information; QuickScan Directories and files info;
Drag 'n' Drop for files manipulation - easy way to work for Mouse Fans;
Hot keys, colors, sizes, fonts and panels layout easy customization
with XNCSetup make XNC very usefull tool. 

Short List of XNC functions:

 -Files execution in built in rxvt or in background
 -Files selection/deselection manually or by mask
 -Copy/Move/Delete files/directories (recursivly)
 -View files in ASCII/HEX/GIF/JPEG/TIFF/PCX/XBM/XPM/TGA/BMP/PhotoCD formats
 -Edit files in ASCII/HEX formats
 -Change owner & attributes of files
 -Creating directories
 -Archieve files copy/del/edit/view/execute with Virtual File System
  supported archieves: TAR, TARGZ, RPM, ARJ, ZIP, LHA, RAR
  Subarchieves (archieve inside archive) supported as well. 
 -Find files on Disk File System
 -Hot Dir Bookmark - quick switching to usefull directories
 -Disk summary information
 -Memory information
 More...
 
See 'Docs' dir for complete description (html format). 

--------------------
Installation:
--------------------

 ./configure
 make
 make install
 xncsetup   - Do it under X Window because XNCSetup needs X

-------------------------
Post Configuration:
-------------------------
If XNC already installed and You want change it configuration (colors,sizes,layouts,hot keys)
run 'xncsetup', do as You want then press 'Save' button.

Warning!!!
Each new user, who wants to use XNC must run 'xncsetup' before...

--------------------------
Execution:
--------------------------
After installation and configuration just type 'xnc' and wait some seconds.
If something goes wrong see XNC output messages.
XNC automatically run 'ives' if internal viewer or editor defined.

For use IVES witout XNC type 'ives' then
  for edit file with IVES - 'es filename'
  for view file with IVES - 'vs filename'
  
For run X Northern Launch Pad type 'xnlaunch'.

For faster xnc startup do:

xncloader   - run it just ONCE after you start you X server

xnc         - run it, and it will use faster loading

-------------------------
Additions:
-------------------------

Please mail me name of city and country where you are using xnc,
it's for my collection ;)

to:                        leo@xnc.dubna.su
  
PS: I'm sorry for my English


         (c) Copyright 1996-2001 by Leo Khramov. St-Petersburg, Russia
         