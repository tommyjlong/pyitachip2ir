"""
Control an itach ip2ir gateway using libitachip2ir
"""
from ctypes import *
import os
import fnmatch
import sys
import logging

def findlib():
    dirname = os.path.dirname(__file__)
    for ext in ['so','dylib','dll','pyd']:
        names = fnmatch.filter(os.listdir(dirname),"*itachip2ir*" + ext)
        for name in names:
            try:
                return cdll.LoadLibrary(dirname +"/"+name)
            except OSError:
                pass
    return None
libitachip2ir = findlib()
if libitachip2ir is None:
    raise OSError("Unable to find itachip2ir library")

libitachip2ir.ITachIP2IR_new.argtypes = [c_char_p, c_char_p, c_int]
libitachip2ir.ITachIP2IR_new.restype = c_void_p
libitachip2ir.ITachIP2IR_delete.argtypes = [c_void_p]
libitachip2ir.ITachIP2IR_ready.argtypes = [c_void_p, c_int]
libitachip2ir.ITachIP2IR_ready.restype = c_bool
libitachip2ir.ITachIP2IR_update.argtypes = [c_void_p]
libitachip2ir.ITachIP2IR_addDevice.argtypes = [c_void_p, c_char_p, c_int, c_int, c_char_p]
libitachip2ir.ITachIP2IR_addDevice.restype = c_bool
libitachip2ir.ITachIP2IR_send.argtypes = [c_void_p, c_char_p, c_char_p, c_int]
libitachip2ir.ITachIP2IR_send.restype = c_bool
c_logcb_p = CFUNCTYPE(None, c_char_p)
libitachip2ir.ITachIP2IR_setLog.argtypes = [c_logcb_p]

if sys.version_info >= (3, 0):
    def tochar(data):
        return bytes(data,'utf-8') if data else None
else:
    def tochar(data):
        return str(data) if data else None

import logging
logger = logging.getLogger(__name__)

def logcb(data):
    logger.info(data.decode('utf-8'))

clogcb = c_logcb_p(logcb)
libitachip2ir.ITachIP2IR_setLog(clogcb)

class ITachIP2IR(object):
    def __init__(self, mac, ip, port):
        self.itachip2ir = libitachip2ir.ITachIP2IR_new(tochar(mac), tochar(ip), port)

    def __del__(self):
        libitachip2ir.ITachIP2IR_delete(self.itachip2ir)

    def ready(self, timeout):
        return libitachip2ir.ITachIP2IR_ready(self.itachip2ir, timeout)

    def update(self):
        libitachip2ir.ITachIP2IR_update(self.itachip2ir)

    def addDevice(self, name, modaddr, connaddr, cmddata):
        return libitachip2ir.ITachIP2IR_addDevice(self.itachip2ir, tochar(name), modaddr, connaddr, tochar(cmddata))

    def send(self, device, command, count):
        libitachip2ir.ITachIP2IR_send(self.itachip2ir, tochar(device), tochar(command), count)
