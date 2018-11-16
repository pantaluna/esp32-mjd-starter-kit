#
# Component Makefile
#
# This Makefile should, at the very least, just include $(SDK_PATH)/make/component.mk. By default,
# this will take the sources in this directory, compile them and link them into
# lib(subdirectory_name).a in the build directory. This behaviour is entirely configurable,
# please read the SDK documents if you need to do this.
#
COMPONENT_SRCDIRS := .
COMPONENT_SRCDIRS += minmea

COMPONENT_ADD_INCLUDEDIRS := include
COMPONENT_ADD_INCLUDEDIRS += minmea

COMPONENT_PRIV_INCLUDEDIRS := 

# ***SPECIAL*** minmea library: 
#	Some systems lack timegm.
#	On these systems, you have to build with -Dtimegm=mktime which will work correctly as long the system runs in the default UTC timezone.
CFLAGS += -Dtimegm=mktime
