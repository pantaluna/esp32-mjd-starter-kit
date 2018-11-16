#
# Component Makefile
#
# This Makefile should, at the very least, just include $(SDK_PATH)/make/component.mk. By default,
# this will take the sources in this directory, compile them and link them into
# lib(subdirectory_name).a in the build directory. This behaviour is entirely configurable,
# please read the SDK documents if you need to do this.
#
COMPONENT_SRCDIRS := .
COMPONENT_SRCDIRS += bosch_bmp280

COMPONENT_ADD_INCLUDEDIRS := include
COMPONENT_ADD_INCLUDEDIRS += bosch_bmp280

COMPONENT_PRIV_INCLUDEDIRS := 
