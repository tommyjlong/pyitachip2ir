TachIP2IR Library Modified To Overcome Socket Failures.  
------------------
*For Use With Home Assistant*

Home Assistant has an itach platform that can be used as a 'remote control'.
It uses the pyitachchip2ir library at https://pypi.org/project/pyitachip2ir/ to provide the protocol dependent 
communications and functions. With this, Home Assistant can send IR commands to various products from Global Cache,
and in my case, the GC-100.

However, when using the Home Assistant supplied pyitachip2ir libary on my Ubuntu systems, I would quite often find that there were failures (mostly silent failures at that) when sending IR commands to the GC-100.  After doing some investigation, what I found was that the socket used for sending IR commands simply failed, and sometimes without returning any errors.  I was not able to determine the actual cause of this, but I changed the code to make sure the socket was restarted when sending an IR Command.   There were some other changes too to simplify things such as removing the listening for beaconing as part of device discovery.  I have since adapted this to work with Homeassistant with HAOS which runs Homeassistant core as a Docker container.

How to use this with Home Assistant
----------
Home Assistant stores a shared library version of pyitachchip2ir in python's ``site-packages`` directory, so you'll need to have access to that directory.  g++ is needed to compile the source code so your system will need g++.

For HA installations using Home Assistant OS, we'll need to add a g++ compiler.  I recommend using the SSH & Web Terminal Add-On for this, and ssh into the ssh container `ssh hassio@homeassistant.local` and then add the g++ package using comand: `apk add g++`.  Later on, it can remove it: `apk del g++`.

* Get the pyitachip2ir modified source code.
  Download/Clone this repository `git clone https://github.com/tommyjlong/pyitachip2ir`</br>

  If you are using Homeassistant installed for use with HAOS, I recommend you goto `/share` and clone the repository there.

  cd to the source directory ``cd pyitachip2ir/source``. 
  In the source directory, execute the following command: 

  ``g++ -shared -fPIC IRCommandParser.cpp ITachIP2IR.cpp -o itachip2ir.so``

  This produces a shared library file ``itachipir.so`` that we will use.<br/>

* Goto the `site-packages` used by Home Assistant.
  If you are using Homeassistant with HAOS, you'll need to get inside the HA Docker container. You'll need to have access to the HAOS console.  If you have setup HAOS for ssh, you can `ssh root@homeassistant.local -p22222`, then `docker exec -it homeassistant /bin/bash`.  Then `cd /usr/local/lib/python3.XX/site-packages/` (where XX is the python version).
* Look for a file something like ``itachip2ir.blahblah.so`` (in my case its is ``itachip2ir.cpython-310-x86_64-linux-gnu.so``).  Either delete the file, or simply change the ``.so`` extension to something like ``.sow`` (when HA's Python code starts up it looks for a file named `*itachip2ir*` with extension `.so` thus changing the extension to `.sow` will cause it to not be used).
* Copy the ``itachip2ir.so`` that was discussed earlier to the ``../site-packages/`` directory. 
* Reboot/Restart HA. 

If using Homeassistant on HAOS, every update of Homeassistant core wil undo all of this.  A script `itach_replace.sh` has been provided that can be used to make this a little easier, and the script can be called using a `shell_command` automation called at HA core startup. 


License
-------
    - MIT License is retained from original code.  Contact: alan@lightningtoads.com

