ITachIP2IR library
------------------

A C++ library with python bindings for sending IR commands to an ITach IP2IR gateway.

It can be given an ip address and port to connect to, or it can be given a mac address and will listen for broadcasts from any ITach gateway that matches the mac address.

It gets the IR commands from a command library file that follows the format of:

.. code-block::

    CH1
    0000 006D 0000 0022 00AC 00AB 0015 0041 0015 0041 0015 0041 0015 0016 0015 0016 0015 0016 0015 0016 0015 0016 0015 0041 0015 0041 0015 0041 0015 0016 0015 0016 0015 0016 0015 0016 0015 0016 0015 0016 0015 0016 0015 0041 0015 0016 0015 0016 0015 0016 0015 0016 0015 0016 0015 0041 0015 0041 0015 0016 0015 0041 0015 0041 0015 0041 0015 0041 0015 0041 0015 0689

    CH2
    0000 006D 0000 0022 00AC 00AB 0015 0041 0015 0041 0015 0041 0015 0016 0015 0016 0015 0016 0015 0016 0015 0016 0015 0041 0015 0041 0015 0041 0015 0016 0015 0016 0015 0016 0015 0016 0015 0016 0015 0041 0015 0016 0015 0041 0015 0016 0015 0016 0015 0016 0015 0016 0015 0016 0015 0016 0015 0041 0015 0016 0015 0041 0015 0041 0015 0041 0015 0041 0015 0041 0015 0689

Dependencies
------------
    - CMake 2.8 or Python 2 or 3

How to use
----------
    - Executable command line interface for sending one shot ir commands
        .. code-block::

            ./itachip2ir [ip-of-itach] [itach-listening-port] [ir-commands.txt] [name-of-ir-command] [itach-mod] [itach-conn] [message-count]

    - Python object
        .. code-block:: python

            import pyitachip2ir
            import logging

            logging.basicConfig(level=logging.INFO) # Get log messages from pyitachip2ir

            mac_address = None # If known use this, it will search for the itach
            ip_address = "127.0.0.1"
            port = 4998
            itach = pyitachip2ir.ITachIP2IR(mac_address, ip_address, port)

            with open("tv_commands.txt",'r') as cmds:
                itach.addDevice("tv", 1, 3, cmds.read())

            if not itach.ready(5000): # Search for 5 seconds for device
                raise IOError("ITach not found")

            itach.send("tv", "ON", 1)

License
-------
    - MIT License

Any questions please contact: alan@lightningtoads.com
