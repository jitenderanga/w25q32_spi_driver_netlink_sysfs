# w25q32_spi_driver_netlink_sysfs
This is the Extended version of w25q32 driver using netlink to access driver
This Is the Driver Code for Flash memory w25q32 based on SPI protocol. This driver have been fully tested on beaglebone and interfaced with sysfs. You can edit and make changes as per your need.

If any doubt feel free to contact me via email
# Email: jitenderanga@gmail.com

# Files
1) documentation  : Full Documentaion of the Device and driver working (Start from Here)
2) node           : Device Node 
3) spi.h          : Header File
4) spi.c          : Source File contain the Documented source


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
