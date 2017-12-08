# GKA Data for Enlightenment

Simple Sqlite3 database editor.

Currently supports opening and editing of databases with a single table, with an INTEGER PRIMARY KEY. 

Working towards keyboard friendly input as part of the http://gka.thinkglobally.org project.

## Installation

Install prerequisite packages:

> $ sudo apt-get install build-essential cmake libsqlite3-dev

Note: also require recent efl which is probably not packaged.

### e22

Build from source

### Or use e17 packages

```
sudo apt-get install e17-dev libelementary-dev
```

Copy the handyfiles into place, you only need to do this if the build complains about missing cmake files:
```
sudo mkdir /usr/local/lib/cmake/
sudo cp -r handyfiles/usr-local-lib-cmake/* /usr/local/lib/cmake/
```

## Build

To make install and run from the command line:

> $ mkdir build  
> $ cd build  
> $ cmake ..  
> $ make  
> $ sudo make install  
> $ gka-data-e  

The project also works with CLion and EDI integrated development environments.

## Keyboard navigation

### Main window

Typing regular characters enters into the search box. 
* **Enter** and **Shift-Enter** move forwards and backwards between records that match this search term.
* **Ctrl-n** New file
* **Ctrl-o** Open file
* **Ctrl-e** Exit
* **Ctrl-Shift-N** New record
* **Ctrl-Shift-E** Edit record
* **Ctrl-Shift-D** Delete record
* **Ctrl-l** Edit Label Preferences

### New file

* **Enter/Return** Create new file
* **Escape** Exit without creating a new file

### Open file

Not currently keyboard navigable, efl standard widget

### New record

* **Ctrl-s** Create new record
* **Escape** Exit without creating a new record

### Edit record

* **Ctrl-s** Update record with changes
* **Escape** Exit discarding changes

### Delete record

* **Enter/Return** Delete the selected record
* **Escape** Exit without deletion

### Edit Label Preferences

* **Ctrl-u** Move selected field up
* **Ctrl-j** Move selected field down
* **Ctrl-e** Edit selected field
* **Ctrl-a** Add new selected field
* **Ctrl-d** Delete selected field
* **Ctrl-s** Save label changes
* **Escape** Exit discarding changes

### Add/Edit field

* **Enter/Return** Action add/edit
* **Escape** Exit add/edit
