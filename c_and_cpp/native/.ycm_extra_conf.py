import os, sys
YCM_CONF_DIR = os.path.abspath(os.path.dirname(__file__))
sys.path.append(os.path.join(YCM_CONF_DIR, "../../python"))
import ycm_conf_for_c_and_cpp
from ycm_conf_for_linux_driver import flags as driver_flags

ycm_conf_for_c_and_cpp.flags.extend([ "-D", "COMMPROTO_LITTLE_ENDIAN", "-D", "TEST" ])
driver_flags.extend([ "-D", "TEST" ])

DRIVER_SRC_BASENAMES = [
    "chardev_group",
    "evol_kernel",
    "klogging",
]

def FlagsForFile(filename: str, **kwargs):
#{#
    base_name = os.path.splitext(os.path.basename(filename))[0]
    app_flags = ycm_conf_for_c_and_cpp.flags

    return { "flags": driver_flags if base_name in DRIVER_SRC_BASENAMES else app_flags }
#}#

ycm_conf_for_c_and_cpp.FlagsForFile = FlagsForFile

Settings = ycm_conf_for_c_and_cpp.Settings

