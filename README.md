# w25q32_spi_driver_netlink_sysfs
This is the Extended version of w25q32 driver using netlink to access driver
This Is the Driver Code for Flash memory w25q32 based on SPI protocol. This driver have been fully tested on beaglebone and interfaced with sysfs. You can edit and make changes as per your need.

If any doubt feel free to contact me via email
# Email: jitenderanga@gmail.com

# Files
1) documentation  : Full Documentaion of the Device and driver working (Start from Here)
2) User Manual    : Documentaion for user how NetLink is Being used by driver to access data
3) node           : Device Node 
4) spi.h          : Header File
5) spi.c          : Source File contain the Documented source
6) spi_netlink.c  : Userspace program for reading and writing data
7) lib_spi.c      : Wrapper Libray for Netlink which make user implementaion for user Easy
8) lib_spi.h      : Header file for Library
9) netlink.so     : Dynamic Linked Library for spi_netlink.c for Netlink

Driver Stack:
----------------
##########################################################

      User-Space			//User Send info from here

-------------------------

     Sysfs  | NetLink  (Kern Abstraction)

-------------------------

     SPI client (High Level Driver) 	//Our Driver Layer

-------------------------

     core layer(protocol)
 
-------------------------

     SPI controller
   
-------------------------

     SPI Device (w25q32)

############################################################
