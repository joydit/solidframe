

# Introduction #

First of all, I suppose you are on a linux box.

You need to have the following installed: gcc-c++, cmake and eventually subversion and curl.
(On a Fedora box, the following will suffice: 'yum install gcc-c++ cmake subversion curl'.)

If you use svn to fetch the latest code you can do:

```
work $ svn checkout http://solidframe.googlecode.com/svn/trunk/ solidframe
```

otherwise you can download archive with source snapshot from:

http://code.google.com/p/solidframe/downloads/list

then
```
work $ tar -xjf solidframe-source-X.Y.Z.tar.bz2
```
so you'll have a folder with **SolidFrame** source code.


Next you'll need to prepare the **SolidFrame** extern library pack:

```
work $ mkdir sf_extern
work $ cd sf_extern
work $ ../solidframe/administration/linux/prepare_extern.sh -a
```

The script will automatically download, extract and build the needed extern libraries.

# Building the example applications #

To build the example applications you need to prepare the build folder. You can do this for different types of builds: release, debug, nolog, optimized, maintain.

  * release - without debug messages, -03, with inlines, no assert
  * debug - with debug messages, -g3, without inlines, with assert
  * nolog - without debug messages, -g3, without inlines, with assert
  * optimized - without debug messages, -O3 -g3, without inlines, with assert
  * maintain - with debug messages, -O3 -g3, with inlines, with assert, all compile warnings active

In this workshop we'll use debug build.

```
solidframe $ ./configure --help
solidframe $ ./configure -f dbg -b debug -e ~/work/sf_extern
solidframe $ ./configure -f rls -b release -e ~/work/sf_extern
```

The above commands will prepare three build environments:
  * build/dbg: makefile based build environment for debug;
  * build/rls: makefile based build environment for release;

Next lets build the proof of concept server:

```
solidframe $ cd build/dbg/example/foundation/concept/main
main $ make
```

If everything went well, the server should build successfully.

Next lets build the clients:

```
main $ cd ../../../client
client $ make
```

The most relevant service of the example server is the alpha service. See below a presentation of alpha protocol and a way to manually test it. Now we use the example clients.

The most interesting example clients for alpha protocol are alphaclient\_p and alphaclient\_s. They are the same except that the first one uses plain connections while the second uses SSL connections.

Before anything we'll need another example application:
```

solidframe $ cd build/dbg/example/system/files
files $ make

# lets build some 100 files with sizes from 4K to 1M

files $ ./create /tmp 4000 1000000 100 1
```

Next we'll need two instances of the proof of concept server:

in one terminal:
```
solidframe $ cd build/dbg/example/foundation/concept/main
main $ ./concept -b 1000
```

in another terminal:

```
solidframe $ cd build/dbg/example/foundation/concept/main
main $ ./concept -b 2000
```

in yet another terminal we should use the clients:

```
solidframe $ cd build/dbg/example/client
```

Now let us use the plain alpha client for local fetching on 10 threads:

```
client $ time ./alphaclient_p 10 localhost 1114 /tmp/00000001 5 2
```

Then lets use it for remote fetching using the second example application instance as peer:

```
client $ time ./alphaclient_p 10 localhost 1114 /tmp/00000001 5 2 localhost 2222
```

Now lets use the alphaclient\_s for SSL connections:

```
client $ time ./alphaclient_s 10 localhost 1124 /tmp/00000001 5 2
client $ time ./alphaclient_p 10 localhost 1124 /tmp/00000001 5 2 localhost 2222
```

**Note the change of port from 1114 to 1124.**

Here is approximately what the client does on every connection it opens to the server:

  1. gets the local or the remote list of files from folder /tmp/00000001 (list or remotelist commands)
  1. for every file in the list it issues a fetch / remote fetch to read the file (fetch command)
  1. it does the previous step twice with 5 milliseconds sleep between commands

# The ALPHA protocol #

The alpha protocol resembles somehow IMAP and has few commands of real interest:
list, remotelist, store, fetch

## ALPHA LIST command ##

`tag list "/absolute/path"`

Lists all items from '/absolute/path' specifying whether they are files or folder, for files, the size is also printed.

## ALPHA REMOTELIST command ##

`tag remotelist /absolute/path/on/peer peer_addr peer_ipc_base_port`

e.g.: `aa remotelist "/tmp" localhost 2222`

Here's what will happen behind the curtain:
  * a signal is created and sent to peer process.
  * there it is executed and it fetches all items under the requested path
  * it returns to the issuer process with a response containing the list of items
  * here the list is written on the connection.

## ALPHA FETCH command ##

`tag fetch "/absolute/path" [peer_addr peer_ipc_base_port]`

It has two forms:

A local fetch form:

`aa fetch "/tmp/00000001/00000006.txt"`

Which does:
  * requests an istream from filemanager for input file
  * when the stream is received, it writes all stream data on connection

And a remote fetch form:

`aa fetch "/tmp/00000001/00000006.txt" localhost 2222`

Which does:
  * Requests from filemanager an iostream for a temporary or memory file
  * when received the stream, it issues a masterfetchsignal to the peer
  * on the peer the master fetch signal requests an istream to the requested file from filemanager
  * when received the stream the masterfetchsignal sends the issuer process a response containing the size of the stream and at most the first 1MB of file data, which on deserialization on issuer process is written on the temporary file
  * if there is data on the file not sent to the issuer, the masterfetch signal waits for slavefetchsignals
  * then the connection, if there is still file data to request from peer, it creates a slavefetchsignal for the next 1MB of file data which will be written at the end of the temp file.
  * also the connection starts sending to the client the file data from the temp file.
  * continues with the previous 3 steps until all file data is sent to the client, using rotationally first MB of temp file to send data to the client and the second MB to receive data from peer, then the first MB to receive data from peer and the second MB to send to the client.

As you'll see, the performance decreases on remote fetch at half of that of the local fetch.

# Closing #

On this page you've learned how to have a working copy of the **SolidFrame** framework, and learned something about its example applications, especially the proof of concept server. You've also learned how to make a small performance test. There you have it, now brace yourself for the interesting part - start using the framework for development [Develop](Develop.md).