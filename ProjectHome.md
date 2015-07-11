# **SolidFrame** has for some time a new home on GitHub #

---



**SolidFrame** is an experimental C++ framework intending to offer tools for building scalable, distributed, client-server applications.

## Supported platforms ##
  * Linux
  * FreeBSD
  * MacOSX
  * Windows - partial support with the back-end based on boost.

## License ##
  * <a href='http://www.boost.org/LICENSE_1_0.txt'>Boost Software License - Version 1.0 - August 17th, 2003</a>

## Downloads ##
  * <a href='https://googledrive.com/host/0B5K3e-G48sSFRzlsdW9HWnVyeUU'>Download location</a>**.
  ***[Releases](http://code.google.com/p/solidframe/wiki/Releases)*****NOTE:**Downloads tab above is deprecated.**

## Content ##
  * Build system based on <a href='http://www.openssl.org/'>CMake</a> which allows easy integration of new applications based on the framework.
  * System library which wraps up threads, synchronization, thread specific, file access, socket address, debug logging engine etc.
  * Asynchronous notification engine.
  * Asynchronous communication engine UDP and plain/secure (via <a href='http://www.openssl.org/'>OpenSSL</a> library) TCP.
  * Asynchronous multiplexed IPC (Inter Process Communication) / RPC (Remote Procedure Call) engine, over UDP.
  * Buffer oriented, asynchronous ready, binary serialization/marshaling engine - for binary protocols like the one used by IPC.
  * Buffer oriented, asynchronous ready, text protocol engine (parser and response builder) for protocols like IMAP, POP, SMTP etc.
  * Implementation of Fast-Multi-Paxos consensus algorithm.
  * Shared object store with asynchronous read/write access.
  * Shared file store with asynchronous read/write access.
  * Audit/log engine completed with a debug logging engine.
  * Doxygen documentation.
  * Example applications.

For more information see this **[wiki page](http://code.google.com/p/solidframe/wiki/Overview)**.