

# 1.0.1439-preview1 #

## Notes ##

  * The API has major changes.
  * There is no documentation at the moment.
  * Tested on Fedora Linux 20

## Changes ##

  1. DONE:IPC gateway - relay IPC data through TCP for site-to-site communication.
  1. DONE: Move everything under solid and solid::frame namespaces
  1. DONE: Refactor solid::frame::Manager (ex. foundation::Manager)
  1. DONE: New binary protocol for client-server apps
  1. DONE: A distributed chat example (using the binary protocol) along with a stress client.
  1. DONE: Move the entire code under more liberal boost 1.0 license
  1. DONE: better response types - no more BAD/OK/NOK
  1. DONE: frame::shared::Store - asynchronous read/write lock.
  1. DONE: Refactor frame::file::Store to use frame::shared::Store
  1. DONE: Proper file rotation for debug
  1. DONE: Per service debug log level mask support and register all services on init

# 0.9.1084 #

  * Refactor the socketaddress interface.
  * Fix foundation::file library
  * Small improvements in ipc library

# 0.9.977 #

  * OS: improved support for PC/Free-BSD and MacOS
  * OS: partial support for windows (with help from boost library) - everything but foundation.
  * IPC: Add support for authenticating the ipc sessions.
  * IPC: Find a better way to disconnect after a failed authentication
  * IPC: Add support for sending authentication commands while in authenticating state. Commands with AuthenticationFlag? set.(NEEDS proper testing!!!!)
  * IPC: Check that a failed authentication, raises the objects waiting for commands to be sent.
  * IPC: Add support for compressing buffers.
  * IPC/Serialization: One should be able to register the same structure/class multiple times, every time being constructed on creation with a different constructor. Also we need to be able to specify the position on which the map will be registered.
  * Serialization: add support for specific objects
  * Serialization: Improve binary serialization performance and create better test applications.
  * Serialization: Add support for pushUtf8(std::string&) to binary::serialization - the string will be serialized as null terminated data, i.e. no size.
  * Serialization: Add support for limits to binary::serialization. Add the following methods and changes to both serializer and deserializer:
    1. constructor(const Limits `*`pdefaultlimits);
    1. resetLimits(); //reset all limits to default
    1. pushStringLimit(uint32(256)) / pushStringLimit(), reset to default.
    1. pushStreamLimit(uint64(200000000)) / pushStreamLimit(), reset to default.
    1. pushContainerLimit(uint32()) - pushContainerLimit(), reset to default.

  * Foundation: Try to optimize the memory usage for foundation::Object, related to the signal queue, by moving the queue onto the foundation::Service.
  * System: Optimize specific memory cache by dropping usage of queues and using a simple linked list.
  * Find a fix for thread safe static member within static function.

# 0.9.850 #

  * Added support for kqueue enabling support for FreeBSD and MacOSX (tested on MacOSX 10.6)
  * Added support for real time and monotonic time in system/TimeSpec

# 0.9.824 #

  * The biggest addition is the new distributed/consensus/Object enabling support for distributed replicated consensual objects - implementing Fast Multi PAXOS algorithm.
  * Big fix in ipcsession.
  * Improved objectselector and fixed aioselector.
  * Added HAVE\_CPP11 compilation flag and cmake check.
  * Incipient windows build support.