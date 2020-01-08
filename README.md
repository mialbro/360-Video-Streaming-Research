# 360-Degree Video Streaming Research

## Prerequisites:
  1. Place gop video files in a folder called video\_files.
  2. Place gop instruction file in gop/gop\_data
  3. Create folder called received on server to hold received files.
 ## **Server**:
    g++ -std=c++11 server.cpp
    ./a.out
 ## **Client:**
    g++ -std=c++11 client.cpp gop.cpp
    ./a.out

## Client Overview:
  Responsible for sending Group-of-Picture tiles to specified source
## Setup:
Read GOP instruction file: gop/gop/gop_data
- For each set of Group-of-Picture:
  - For each tile:
    - Store the value (priority of tile), row (specifies the group that the tile belongs to based on transfer speeds) \*not to be confused with the physical position of the tile, and position (specifies the physical row/column where the tile lies) into a multidimensional array.
   - Sort this array by the tile&#39;s value (the tile&#39;s priority). \*After this, the tiles with the highest priority will be in the front; therefore, they will be sent first. The tiles that we will not send wil be at the back of the array.
   - Create multidimensional arrays to hold the filenames and headers for each tile. \*The **header** is sent with the file and includes information about the tile being sent: position, name, and size.
      
### Data Transfer:
- For each Group-of-Pictures:
  - For each tile:
    - Break up the file, sending 64000 bytes at a time, untill you get to the last portion of the file, which may be less.
    - Append the header to first chunk of th file. The header has the following structure: gop#-row-column-size. ex: 01-03-04-55000.
## Throughput
- Continuously calculate the time necessary to send the file and perform necessary operations to get the transfer rate in Megabits per second (throughput).
- Use throughput to determine which set of tiles to send.

## Server Overview
Receives the gop tiles
### Setup:
- For each Group-of\_pictures:
    - For each tile:
      - Listen for the first portion of the file which will also include the header.
      - Extract filename, filesize, and the tile position information from the header.
      - Receive the rest of the file
