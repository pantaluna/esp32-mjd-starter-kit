#
# Component Makefile
#
# This Makefile should, at the very least, just include $(SDK_PATH)/make/component.mk. By default,
# this will take the sources in this directory, compile them and link them into
# lib(subdirectory_name).a in the build directory. This behaviour is entirely configurable,
# please read the SDK documents if you need to do this.
#
COMPONENT_SRCDIRS := .
COMPONENT_SRCDIRS += bosch_bme280

COMPONENT_ADD_INCLUDEDIRS := include
COMPONENT_ADD_INCLUDEDIRS += bosch_bme280

COMPONENT_PRIV_INCLUDEDIRS := 

# ***SPECIAL*** Bosch BME280 Driver: Regarding compensation functions for temperature, pressure and humidity we have two implementations.
#		a. Double precision floating point version
#		b. Integer version
#	By default, the integer version is used in the API!
#	==> If the user needs the floating point version, the user has to uncomment BME280_FLOAT_ENABLE macro in bme280_defs.h file, 
#	or add that DEFINE to the compiler flags (CFLAGS).
CFLAGS += -DBME280_FLOAT_ENABLE
