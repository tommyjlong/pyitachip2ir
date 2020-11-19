TachIP2IR Library Modified To Overcome Socket Failures.  
------------------
*For Use With Home Assistant In a Python Environment*

Home Assistant has an itach platform that can be used as a 'remote control'.
It uses the pyitachchip2ir library at https://pypi.org/project/pyitachip2ir/ to provide the protocol dependent 
communications and functions. With this, Home Assistant can send IR commands to various products from Global Cache,
and in my case, the GC-100.

However, when using the Home Assistant supplied pyitachip2ir libary on my Ubuntu systems, I would quite often find that there were failures (mostly silent failures at that) when sending IR commands to the GC-100.  After doing some investigation, what I found was that the socket used for sending IR commands simply failed, and sometimes without returning any errors.  I was not able to determine the actual cause of this, but I changed the code to make sure the socket was restarted when sending an IR Command.   There were some other changes too to simplify things such as removing the listening for beaconing as part of device discovery.

How to use this with Home Assistant
----------
First of all, the target for this is for use with **HA in a Python environment**.
Home Assistant stores a shared library version of pyitachchip2ir in python's ``site-packages`` directory, so you'll need to have access to that directory.

* Download/Clone this repository, and cd to the source directory ``cd ../source``. For an Ubuntu based system execute the following command: 

  ``g++ -shared -fPIC IRCommandParser.cpp ITachIP2IR.cpp -o itachip2ir.so``

  This produces a shared library file ``itachipir.so`` that we will use.<br/>

  If your system runs on a 64bit AMD/Intel CPU, you should be able to use the itachipir.so file already in the repository.  To know what architecture/CPU you system runs on, execute the command `uname -m` and if it shows `x86_64` then you should be able to use the already supplied itachipir.so file.
* cd into the ``../site-packages/`` directory used by Home Assistant.
* Look for a file something like ``itachip2ir.blahblah.so`` (in my case its is ``itachip2ir.cpython-37m-x86_64-linux-gnu.so``).  Either delete the file, or simply change the ``.so`` extension to something like ``.sow`` (when HA's Python code starts up it looks for a file named (wildcard)itachip2ir(wildcard) with extension .so).
* Copy the ``itachip2ir.so`` that was discussed earlier to the ``../site-packages/`` directory. 
* Reboot/Restart HA. 

Be aware that if the itachip2ir library in PyPi changes, HA may in turn bring in an updated itachip2ir.blahblah.so file on the next HA upgrade, so reapplying this same fix may or may not work.


License
-------
    - MIT License is retained from original code.  Contact: alan@lightningtoads.com

