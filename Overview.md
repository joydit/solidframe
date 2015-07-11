

# Introduction #

Regarding the size of a C++ project even at an early stage,  it needs a strong framework to help build upon.

By framework I mean:

  * an easy to use, cross platform build system;
  * a powerful set of libraries and concepts to help design and write applications

Within any team there is always diversity:
  * one likes using Eclipse,
  * another likes MS Visual Studio,
  * others love KDevelop.

Modern build systems like **CMake** (http://www.cmake.org/) or **SCons** (http://www.scons.org) can generate project files for different IDEs. So we should use them.

But, how? What if you have two or more projects?

What will you do? Copy/paste the CMake/SCons files and adapt them to the new project?

Well, no! You can use multi repository projects.
You can have a base repository containing the framework (the build system and the base libraries) and have the real applications repositories plug-into the framework's build system.

Also, one can think of a mechanism of promoting code from "upper" repository to the base one.

This is how the **SolidFrame** framework is designed. It uses CMake as build system, it has libraries and groups of libraries to help write from simple to the most sophisticated cross-platform C++ applications.

You can add new application/library repositories into the _application_ / _library_ directories and run `configure` to have the application/library plugged into the build system. You can also instruct the build system (using the configure script) to create project/solution for the preferred IDE and start coding; or you can instruct it to create console based builds (for make/nmake).

Also the framework is designed to easily plug in external libraries. SolidFrame by default depends of boost (for some of the examples) and openssl, but the architecture was successfully tested to work with other libraries like: Qt for GUI applications, ICU for internationalization and Xerces for xml parsing.

**SolidFrame's** main goal is to enable the writing of powerfull cross-platform server side distributed applications, but can be used to easily write any kind of C++ application.

So what do we need for writing C++ applications:
  * an easy to extend, cross-platform, build system;
  * wrappers for different system concepts: threads, mutexes, conditions, sockets, socket addresses etc.;
  * a debug log engine to log debug messages (it might be old fashion but THEY ARE REALLY USEFUL);
  * (might need) more sophisticated concepts like thread pools;
  * (might need) serialization algorithms to allow easy marshaling objects;
  * (might need) support for implementing text based internet protocols;
  * (might need) asynchronous signaling and communication (secure and plain), for powerful servers;
  * (might need) ways of passing signals from a process to another, for distributed applications;
  * (might need) an audit (logging) engine.

**SolidFrame** offers all this and more. It has a neat design, nice concepts like Pseudo Active Objects and a **solid** implementation. It has lots of example applications, including a central proof of concept server to show most of the offered concepts. Also there are some clients written to stress the proof of concept server for profiling purposes.

Here are some of the nicest things it offers:

  * an re-entrant serialization engine allowing the use of fixed sized buffers to serialize/marshal objects of any size (one can even serialize files!) - see below for details;
  * a text protocol re-entrant parser and response builder - to also be used with fixed sized buffers;
  * a nice asynchronous signaling engine - based on Pseudo Active Objects;
  * a powerful filemanager to allow non conflicting access to files from within the same process. The file will be sent as a signal to a requesting pseudo-active object when the request does not conflict with others (e.g. no writer while there are readers);
  * a very nice Inter Process Communication / Remote Procedure Call engine, which allows commands to be executed on a different process/machine and have the response returned back to the sender;
  * a powerful asynchronous communication engine, TCP (plain and SSL) and UDP.

## Some words about the build system ##

The build system is based on CMake with a simple shell script to easily configure it. On `*`nix use ./configure on Windows configure.bat then configure.ps1. With this basic script, you can:
  1. plug application or library repositories into the build system.
  1. integrate external libraries into the build system.
  1. prepare folders different types of console builds: debug, release, no-log, maintain;
  1. create project files for different IDEs.
  1. build the documentation (doxygen is needed).

In the following paragraphs I will present all libraries and groups of libraries from **SolidFrame**.

# The **"system"** library #

It contains system wrappers for: _thread_, _mutex_, _condition_, _semaphore_, _timespec_ (time in seconds and nanoseconds), _devices_ (file descriptors: files, sockets), _socket addresses_, _thread specific data_. It also contains typedefs for basic types, some framework wide used enums and a powerfull debug logging engine. The debug logging is deactivated on release builds. The threading and the debug engine need special initializations to be made at the start of your application:

```
Thread::init();
```

and for debugging, something like:

```
#ifdef UDEBUG
	{
	string dbgout;
	Dbg::instance().levelMask(p.dbg_levels.c_str());
	Dbg::instance().moduleMask(p.dbg_modules.c_str());
	if(p.dbg_addr.size() && p.dbg_port.size()){
		Dbg::instance().initSocket(
			p.dbg_addr.c_str(),
			p.dbg_port.c_str(),
			p.dbg_buffered,
			&dbgout
		);
	}else{
		Dbg::instance().initFile(
			*argv[0] == '.' ? argv[0] + 2 : argv[0],
			p.dbg_buffered,
			3,
			1024 * 1024 * 64,
			&dbgout
		);
	}
	cout<<"Debug output: "<<dbgout<<endl;
	dbgout.clear();
	Dbg::instance().moduleBits(dbgout);
	cout<<"Debug modules: "<<dbgout<<endl;
	}
#endif
```

To initiate the debug logging to output either to _a file_, to _stderr_ or to _a socket_ connected to a server (E.g. on `*`nix I use "nc -l" to view the debug log messages.)

Here is an example of how the debug log should be used:

```
	idbg("Built on *SolidFrame* version "<<SG_MAJOR<<'.'<<SG_MINOR<<'.'<<SG_PATCH);
```

# The **"utility"** library #

This library depends on **system** and defines concepts like: _streams_, _threadpool_ (workpool). It also contains some utility classes for concepts like: _stack_, _queue_, _list_. Their interfaces are similar to those of STL, but implementation is twice faster at run time. Another interesting utility class is MutualStore that can be used when having objects (like mutexes) shared by some other groups of objects from a container. To be more explicit here's the situation it is used:

  1. We have an indexed container of objects (vector or deque). Every object needs an associated mutex. We also need to limit the number of used mutexes.
  1. So, we want that N objects to share the same mutex.
  1. We want at most M = Mc x Mr mutexes, where Mc is the number of mutexes allocated once (c for columns) and Mr is the number of rows.
  1. Using the object's index within the container, a mutex is given.

When the index exceeds N x M, the mutexes returned start again from 0x0, and a mutex will guard more that N objects.

# The **"algorithm"** library group #

There are two libraries: **protocol** for implementing internet text protocols, and **serialization** for re-entrant serialization.

As you've learned, in **SolidFrame** framework, the communication is asynchronous, so although it simplifies some things, it complicates a little the parsing of the data received on the communication channel and the building of the response. The "protocol" library helps with that: re-entrant parsing with error handling and re-entrant response builder. I've designed the protocol library having the experience of implementing the IMAP protocol so, as you'll see from the foundation **alpha example protocol** which resembles IMAP, the library is quite easy to use. By re-entrant I mean, for example in case of parsing, that you don't have to feed the parser all the data, you give the parser the data you have, and it will inform you if it has finished parsing or if it needs more data.

Same thing with the **serialization** library. Although not as complete as, let's say, boost::serialization, it has the major advantage of being re-entrant. For the moment it only features binary serialization, written specifically for the IPC engine. It can serialize:

  * basic types,
  * (abstract) objects as pointers and references,
  * STL::containers of objects and of pointers,
  * and, yes, it can serialize streams (e.g. file streams) etc.

**NOTE:** When used with pointers to objects, great care must be taken, as no runtime checking is made.

# The **"audit"** library group #

For now the only libraries implemented are for logging (**audit\_log** **audit\_log\_manager**), the _reporting_ library being planned for a future release.

The **audit\_log** library provides an interface for a log client. You initiate the log client, connecting it to a log manager and feed it log lines. The interface resembles the debug engine. Here is an excerpt of initiating both the client and the server (usually these are not done within the same process):

```
	pipe(pairfd);
	//the log manager init
	audit::LogManager lm;
	lm.start();
	lm.insertChannel(new DeviceIOStream(pairfd[0], pairfd[1]));
	lm.insertListener("localhost", "3333");
	Directory::create("log");
	lm.insertConnector(new audit::LogBasicConnector("log"));
	
	//the log client init
	Log::instance().reinit(argv[0], 0, "ALL", new DeviceIOStream(pairfd[1],-1));
```

And here's an example of usage:

```
	ilog(Log::any, 0, "some message "<<some_value<<" some words "<<another_value);
```

# The **"foundation"** library group #

This is the central group of libraries on the framework. It defines most of its powerful concepts.
The central concept is the **Pseudo-Active Object** (PAO), which is an object residing both within a static container (a Service) and in an active pool (a thread pool). All services reside within Manager and are also PAOs themselves. The manager is used to send different signals to the objects. The only way to access directly an object is through a visitor, the rest of interaction is through signals.

Every object has an unique id which uniquely identifies the object in both space (indexed access) and time (one can safely send a signal to a deleted object). This unique id has two parts, an index part to locate the object starting from the manager (contains the index of the service and the index within the service) and an unique number incremented on every release of a position within service.

The role of active containers (SelectPools) is to keep objects and provide them with processor time on certain events like: signals, timeouts, IO completion events.

There are three foundation libraries and group of libraries: **foundation\_core**, **foundation\_aio** group and **foundation\_ipc**.

## The **"foundation\_core"** library ##

It defines the main concepts: the base class for PAOs, the services, schedulers and the manager.

## The **"foundation\_file"** library ##

This library contains the file::Manager and some additional classed for high level stream based file access management.

Suppose you have different objects (PAOs) that need to access a certain file. Some access modes are incompatible: one cannot write while there are readers, only a single writer at a time, and no readers while there are writers.

Requesting a stream to a file from the file manager, will either return immediately with success and the stream or with failure or with a "pending" response, when the requested stream conflicts with other existing stream(s). In the last case, the filemanager will send the stream to a PAO using a signal. Also it may happen for filemanager to signal an error.

The file manager offers support for temporary and in-memory files. For a very nice example of both, the way it should be used and its power, see in the foundation/concept example, the alpha fetch command, especially how the remote fetching is implemented (example/foundation/concept/alpha/src/alphacommands.cpp).

## The **"foundation\_aio"** library group ##

It offers support for asynchronous TCP (plain and secured with SSL) and UDP IO. There are two libraries in the group: **foundation\_aio**, **foundation\_aio\_openssl**.

The **foundation\_aio** library offers the _foundation_::_aio_::_Selector_ (to be used with foundation::SelectPool), the active container for PAOs needing IO support. An object needing AIO support, can have one or multiple channels (e.g. for easy implementing proxies and chat rooms).

The **foundation\_aio\_openssl** is a wrapper for OpenSSL (http://www.openssl.org) library.

## The **"foundation\_ipc"** library ##

This library offers support for remote execution of signals. It uses UDP for communication and binary serialization for signals. It has keep-alive support; which means that sending a command which has to generate a response, the sender side IPC module will periodically send keep alive data to detect peer disconnection. If the disconnection is detected, the PAO waiting for the response, will be signaled with an error. This is useful when waiting for a response, you cannot approximate how long will it take for the peer process to build that response, so you will only wait for the response or an error from the ipc module.

It uses UDP (an aio UDP socket) for "talking" with multiple peer processes. More over, because signals can be quite long (especially if they contain streams) the signals being sent, on the same time, to a peer process are multiplexed: no more than N data buffers are sent continuously for a signal, bigger signals being re-queued.
Because it uses UDP, I had to add many of TCP capabilities like: in-order buffer receive, resending not-received buffers etc.

The stress tests I've performed show the same IO transfer speed as TCP.

# Closing #

In this page you've learned basic knowledge about **SolidFrame**: what it is, what it offers, about its core concepts and its libraries. Next [Testing](Testing.md) you'll learn about how to compile and test the **SolidFrame** example applications.