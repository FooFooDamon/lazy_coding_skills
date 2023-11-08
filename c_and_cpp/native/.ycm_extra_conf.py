import os, sys
YCM_CONF_DIR = os.path.abspath(os.path.dirname(__file__))
sys.path.append(os.path.join(YCM_CONF_DIR, "..", "..", "python"))
sys.path.append(os.path.join(YCM_CONF_DIR, "python"))
import ycm_conf_for_c_and_cpp
from ycm_conf_for_linux_driver import flags as driver_flags

extra_inc_dirs = []
if os.path.exists(os.path.join(YCM_CONF_DIR, "c_and_cpp")):
#{#
    extra_inc_dirs.append("-I" + os.path.join(YCM_CONF_DIR, "c_and_cpp", "native"))
#}#
if os.path.exists(os.path.join(YCM_CONF_DIR, ".paths")):
#{#
    extra_inc_dirs.extend([
        "-I" + os.path.join(YCM_CONF_DIR, ".paths", i, "c_and_cpp", "native")
        for i in os.listdir(os.path.join(YCM_CONF_DIR, ".paths"))
    ])
#}#

ycm_conf_for_c_and_cpp.flags.extend([ "-D", "COMMPROTO_LITTLE_ENDIAN", "-D", "TEST" ])
ycm_conf_for_c_and_cpp.flags.extend(extra_inc_dirs)

driver_flags.extend([ "-D", "TEST" ])
driver_flags.extend(extra_inc_dirs)

DRIVER_SRC_BASENAMES = [
    "chardev_group",
    "evol_kernel",
    "klogging",
    "test_inline_klogging",
]

def FlagsForFile(filename: str, **kwargs):
#{#
    base_name = os.path.splitext(os.path.basename(filename))[0]
    app_flags = ycm_conf_for_c_and_cpp.flags

    return { "flags": driver_flags if base_name in DRIVER_SRC_BASENAMES else app_flags }
#}#

ycm_conf_for_c_and_cpp.FlagsForFile = FlagsForFile

Settings = ycm_conf_for_c_and_cpp.Settings

