# Cord 
Cord is a general purpose discord library written in C (99). The goal
of the project is to provide an easy to use library for creating applications
that take advantage of the Discord API

## Features
* Easy to use API
* Built-in persistent store/model creation
* Support for async I/O by utilizing libev

## Example
The following example bot uses cord to create a simple echo bot
`#include <cord.h>`
For a more advanced example see Discordia the discord bot i wrote using cord

## Documentation
The doxygen generated documentation can be found inside the doc directory

## Building
To build the library you first need to download the dependencies
`sudo apt update`
`sudo apt install libjansson-dev lubssl-dev`
Clone the repository
Create the build and bin directories
``

## Installing
See building instructions
Navigate to the the root directory and run
`sudo make install`
To uninstall simply run
`sudo make uninstall`
