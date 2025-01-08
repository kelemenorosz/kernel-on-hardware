## Kernel on hardware

* ``git clone`` this repository
* Navigate to ``/``
* Set ``CROSS_COMPILER`` environment variable to the cross-compiler install directory 
* Create ``/build`` directory
* Navigate to ``/build``
* Run ``cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake ../``
* Run ``make``
* Run ``make install``
* Navigate to ``/``
* Run ``copy.sh -f <USB_PATH>``

## Remarks

To be used with the github.com/kelemenorosz/bootloader-writer/ repository