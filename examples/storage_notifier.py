#!/usr/bin/env python2
# -*- coding: utf-8 -*-

import dbus
import gobject
import socket
from dbus.mainloop.glib import DBusGMainLoop

title = "New device found"
twmnd_port = 9797
twmnd_host = "127.0.0.1"


class DeviceAddedListener:
    def __init__(self):
        self.bus = dbus.SystemBus()
        self.hal_manager_obj = self.bus.get_object(
                                              "org.freedesktop.Hal",
                                              "/org/freedesktop/Hal/Manager")
        self.hal_manager = dbus.Interface(self.hal_manager_obj,
                                          "org.freedesktop.Hal.Manager")
        self.hal_manager.connect_to_signal("DeviceAdded", self._filter)

    def _filter(self, udi):
        device_obj = self.bus.get_object("org.freedesktop.Hal", udi)
        device = dbus.Interface(device_obj, "org.freedesktop.Hal.Device")

        if device.QueryCapability("volume"):
            return self.do_something(device)

    def do_something(self, volume):
        device = volume.GetProperty("block.device")
        label = volume.GetProperty("volume.label")
        #fstype = volume.GetProperty("volume.fstype")
        #mounted = volume.GetProperty("volume.is_mounted")
        #mount_point = volume.GetProperty("volume.mount_point")
        try:
            size = volume.GetProperty("volume.size")
        except:
            size = 0

        #print "  device_file: %s" % device_file
        #print "  label: %s" % label
        #print "  fstype: %s" % fstype
        size = float(size) / 1024**3
        content = str(label) + '(' + device + ' - ' + ("%.2fGb" % size) + ')'
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect((twmnd_host, twmnd_port))
        mount_dir = "/media/"
        if len(label):
            mount_dir = mount_dir + label
        else:
            mount_dir = mount_dir + device.split('/')[2]

        s.send("<root>" + \
               "<title>" + title + "</title>" + \
               "<content>" + content + "</content>" + \
               "<ac>" + "</ac>" + \
               "</root>")

if __name__ == "__main__":
    DBusGMainLoop(set_as_default=True)
    loop = gobject.MainLoop()
    DeviceAddedListener()
    loop.run()
