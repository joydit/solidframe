# Introduction #

Today I have just found out what was the problem with solidframe/examples/extern/boost/asio/proxyserver.cpp which led to poor performance on Linux.
I have found out that, the sockets, although used asynchronously, they were blocking. All was needed was to make them non blocking:

```
    tcp::socket::non_blocking_io non_blocking_io(true);
    socket_.io_control(non_blocking_io);
```

That nasty problem prevented me to do a head to head performance comparison with boost::asio.

# Setting up the test environment #

Of course I've immediately set up the test environment (a Fedora 13 machine with an AMD3000+ x64):
  * **Programs involved**: solidframe/examples/client/alphaclient\_p, solidframe/examples/foundation/main/concept and solidframe/examples/extern/boost/asio/proxyserver.
  * **The scenario**: on the same machine start the servers in three different consoles:
    1. `./concept` - the file server, serves files (10 files with sizes from 4KB to 8.8MB) using concept::alpha protocol.
    1. `./concept -b 2000` - solidframe asio proxy on port 2115.
    1. `./proxyserver 3115 localhost 1114` - boost::asio (boost 1.44.0) proxy implementation on port 3115.
  * **The test**: I used alphaclient\_p with 50 threads, testing head to head solidframe proxy with boost proxy. Every thread do:
    * request the list of files (alpha LIST command)
    * fetches every file in the list two times (alpha FETCH command).

Here is how alphaclient\_p was launched for solidframe proxy:
```
time ./alphaclient_p 50 localhost 2115 ~/tmp/00000001 1 2 "" 1114
```

and here is for boost proxy:
```
time ./alphaclient_p 50 localhost 3115 ~/tmp/00000001 1 2
```

# Results #

| **Test** | **Time(s)** | **Max Avg Transfer Rate(KB/s)** |
|:---------|:------------|:--------------------------------|
|          | 74.262      |  60205                          |
|          | 74.757      |  60300                          |
|          | 73.770      |  60600                          |
| Boost:asio | 74.263      |  60368,33                       |
|          | 71.257      |  62700                          |
|          | 70.765      |  62800                          |
|          | 70.759      |  62800                          |
| SolidFrame::aio| 70.927      | 62766,66                        |


# Conclusions #

As you can see in the table, solidframe::aio is doing quite well with more than 3 seconds faster test completion.

Note that this test was made on local-loop, with all the files cached in memory (by Linux). It is still conclusive as it shows better usage of system resources (memory, cache, processor etc).

I'll try to repeat the test on other machines and eventually on a gigabit network, so ...

**STAY TUNED!!!**