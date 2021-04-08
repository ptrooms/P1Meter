Import('env')
import os
import shutil

# copy P1Meter/P1OverrideSettings.sample.h to P1Meter/P1OverrideSettings.h
# if not yet existing
if os.path.isfile("P1Meter/P1OverrideSettings.h"):
    print ("*** use provided user_config_override.h as planned ***")
else: 
    shutil.copy("P1Meter/P1OverrideSettings_sample.h", "P1Meter/P1OverrideSettings.h")
