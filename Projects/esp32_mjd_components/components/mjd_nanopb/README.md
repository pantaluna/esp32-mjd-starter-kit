# ESP32 MJD Nanopb component

This is a custom component based on ESP-IDF for the ESP32 hardware from Espressif.

It includes the common C files of the Nanopb library v0.3.9.2.
It also declares Nanopb specific project-wide compilation directives (-D) in Makefile.projbuild

## SOP Make This Custom Component [once]

* Download & extract the Windows64 version of NanoPb
  	SOURCE:   https://jpa.kapsi.fi/nanopb/download/nanopb-0.3.9.2-windows-x86.zip
  	TARGET: \_downloads\

* Copy the following files from the extracted root folder to the root folder of this component:
  pb_common.c
  pb_decode.c
  pb_encode.c
  include\pb.h
  include\pb_common.h
  include\pb_decode.h
  include\pb_encode.h

* Create Makefile.projbuild in the root folder of this component (the source is the documentation).

* Create component.mk in the root folder of this component (the source is the documentation). No special settings are required but the file must exist!

## Example ESP-IDF project
ncliot_http_client
