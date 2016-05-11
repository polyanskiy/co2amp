# Installation Guide for co2amp on Ubuntu 14.04

## Download co2amp
    git clone https://github.com/polyanskiy/co2amp.git

## Install Qt5.5.1 
An installation script can be found at https://www.qt.io/download/.
Make sure you find the path to the associated qmake. 
Multiple versions of qmake already be installed. 
It might be best to alias qmake to the version you want to use. 

## Install gnuplot
Version 4.6 from Ubuntu 14.04 repositories worked.
    
    sudo apt-get install gnuplot

This is needed to use the default plots. 
The calculation part of co2amp will work without gnuplot but the will report command line errors.

## Install 7zip 
The version from Ubuntu 14.04  repositories worked.
    
   sudo apt-get install p7zip-full

This is needed for saving simulation data. 

## Compile co2amp-core
Used QMake 3.0 to create co2amp-core makefile. 
Then use make to compile.
    
    qmake -makefile co2amp-core.pro -o Makefile && make 

You can ignore the warnings about fread().

## Add co2amp-core to search path 
Add the following line to your .bashrc file. 
    
    export PATH=/path/to/directory/containing/co2amp-core:$PATH

Now the co2amp GUI can find co2amp-core. 
It may be better to copy the compiled version of co2amp-core to a directory such as ~/bin or /usr/local/bin and ensure they are in the search path. 
It is noteworthy that co2amp GUI runs out of the temporary directory /tmp/co2amp/xxxxx/ and if co2amp-core is moved to the /tmp/co2amp/xxxxx/ directory then co2amp calculation will run.

## Compile co2amp GUI 
Use QMake 3.0 to create co2amp-shell makefile and then compile it.

    qmake -makefile co2amp-shell.pro -o Makefile && make

Again you may want to add co2amp to the search path so that it can be run from anywhere. Try /usr/local/bin or ~/bin. 

## Run 

    ./co2amp

Temp files will be created at /tmp/co2amp/xxxxx/. 
Save them or else they will be deleted after the GUI is closed. 





