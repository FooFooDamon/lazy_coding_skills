import os, sys
YCM_CONF_DIR = os.path.abspath(os.path.dirname(__file__))
sys.path.append(os.path.join(YCM_CONF_DIR, "../../python"))
from ycm_conf_for_c_and_cpp import *
flags.extend([ "-I", YCM_CONF_DIR ])

