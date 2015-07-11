

# Introduction #

I suppose you've managed to build the **SolidFrame** example applications, which means that you have a working copy. You'll now learn to develop using **SolidFrame**.

# Creating a KDevelop3 project #

You've already learned the command to build a KDevelop3 project:

```
solidframe $ ./configure -f kdv -g KDevelop3 -b debug -e ~/work/sf_extern
```

In KDevelop3, open the project from: `/path/to/solidframe/build/kdv/SolidFrame.kdevelop`.

That's all, eventually you can configure KDevelop3 to use Subversion for project, and use 4 spaces wide tabs for indentation.


# Creating a KDevelop4 project #

The Kdevelop4 is out for a while now, and the difference from KDevelop3 is that it isn't supported by cmake, but instead it supports cmake based projects. This comes somehow as a disadvantage as some by hand configuration is needed. Here are the steps:

Step 1. Embed application and library folders into project
```
solidframe $ ./configure -c
```

Step 2. Open KDevelop4 and import SolidFrame. Select Project->Open/ImportProject. Go to SolidFrame's folder and select CMakeLists.txt file from there. Push Next, then Finish.

Step 3. If a window with "Kdevelop has found several possible root for your project ...", select SolidFrame's folder path.

Step 4. In the window "Configure a build directory for ...", edit the "Build Directory:" line, and append /kdv4. Press OK.

Step 5. Select Project->Configure Selection.

Step 6. In the "Projects" area on left right-click on solidframe and select "Open Configuration". There you should edit:
  * EXTERN\_PATH to point to where you've built the extern libraries (See [Testing#Introduction](Testing#Introduction.md)).


(Step 7.) Build the main concept example. On the left Projects area, navigate to: solidframe->example->main. Right-Click on "concept" and select "Build". Pretty neat ... isn't it!!!

In the following lines I will present the way to start writing a new **SolidFrame** application then I'll present the proof of concept example application.

# Adding a new application #

**SolidFrame** repository copy contains an empty folder called "application". There's where your application(s) will reside.

So you should do something like:

```
solidframe/application $ svn co https://foobarsuite.googlecode.com/svn/trunk/ foobar
```

to have a repository copy of your application within `"solidframe/application/foobar"`, with no files and no folders. Lets add some (we suppose the foobar contains: a client, a server and some test applications):

  1. first you'll need to create the folders:
```
		application/foobar $ mkdir server && svn add server
		application/foobar $ mkdir client && svn add client
		application/foobar $ mkdir test && svn add test
```
  1. next you'll need the CMakeLists.txt file that will look in every foobar's subfolder:
```
		application/foobar $ cat > CMakeLists.txt
		add_subdirectory (server)
		add_subdirectory (client)
		add_subdirectory (test)
		# CTRL+C
		application/foobar $ touch server/CMakeLists.txt
		application/foobar $ touch client/CMakeLists.txt
		application/foobar $ touch test/CMakeLists.txt
```
  1. then you'll need to add the source files to every foobar subfolder, along with the CMakeLists.txt files.
  1. last you'll need to integrate "foobar" into **SolidFrame**'s build system. This is done by running solidframe/build.sh again:
```
		solidframe $ ./configure -f dbg -b debug -e ~/work/sg_extern
		solidframe $ ./configure -f kdv -g KDevelop3 -b debug -e ~/work/sg_extern
		solidframe $ ./configure -f rls -b release -e ~/work/sg_extern
```

> It will create `application/CMakeLists.txt` which will integrate all subfolders within 'application' into **SolidFrame** cmake build system.

As you can see, you can have multiple **SolidFrame** based projects within 'application'.

# The proof of concept example application #

The Proof Of Concept Example Application (`solidframe/example/foundation/concept`) is a multi service, server offering:
  * an ALPHA service resembling IMAP on syntax;
  * echo service for TCP and UDP;
  * a proxy service.

You've built it and tested it, now lets dig into the code, as shows how the **SolidFrame** libraries should be used.

Lets look first at the main function (`solidframe/example/foundation/concept/main/src/concept.cpp`).


# The main function of the concept server #

```
int main(int argc, char* argv[]){
```

Ignore the sigpipe signal:

```
	signal(SIGPIPE, SIG_IGN);
```

Parse the input parameters using boost\_program\_options:

```
	Params p;
	if(parseArguments(p, argc, argv)) return 0;
```

Initiate the thread support:

```
	Thread::init();
```

Initiate the debug log engine either to log to a socket or to a file

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

Initiate the audit log engine and connect the client with the manager through a pipe:

```
	pipe(pairfd);
	audit::LogManager lm;
	lm.start();
	lm.insertChannel(new DeviceIOStream(pairfd[0], pairfd[1]));
	lm.insertListener("localhost", "3333");
	Directory::create("log");
	lm.insertConnector(new audit::LogBasicConnector("log"));
	Log::instance().reinit(argv[0], Log::AllLevels, "ALL", new DeviceIOStream(pairfd[1],-1));
```

Instantiate a Manager, add a new echo service, a new listener and a new talker to it:

```
	typedef foundation::Scheduler<foundation::aio::Selector>	AioSchedulerT;
	typedef foundation::Scheduler<foundation::ObjectSelector>	SchedulerT;

	//...
	concept::Manager	m;
	SignalResultWaiter	rw;
	int 				rv;
	
	m.registerScheduler(new SchedulerT(*this));
	m.registerScheduler(new AioSchedulerT(*this));
	m.registerScheduler(new AioSchedulerT(*this));
	
	const IndexT alphaidx = m.registerService<concept::SchedulerT>(concept::alpha::Service::create(m));
	const IndexT proxyidx = m.registerService<concept::SchedulerT>(concept::proxy::Service::create());
	const IndexT gammaidx = m.registerService<concept::SchedulerT>(concept::gamma::Service::create());
	
	m.start();
	
	if(true){
		int port = p.start_port + 222;
		AddrInfo ai("0.0.0.0", port, 0, AddrInfo::Inet4, AddrInfo::Datagram);
		if(!ai.empty() && !(rv = foundation::ipc::Service::the().insertTalker(ai.begin()))){
			cout<<"[ipc] Added talker on port "<<port<<endl;
		}else{
			cout<<"[ipc] Failed adding talker on port "<<port<<" rv = "<<rv<<endl;
		}
	}
	
	if(true){//creates and registers a new alpha service
		insertListener(rw, "alpha", alphaidx, "0.0.0.0", p.start_port + 114, false);
		insertListener(rw, "alpha", alphaidx, "0.0.0.0", p.start_port + 124, true);
	}
	
	if(true){//creates and registers a new alpha service
		insertListener(rw, "proxy", proxyidx, "0.0.0.0", p.start_port + 214, false);
		//insertListener(rw, "alpha", alphaidx, "0.0.0.0", p.start_port + 124, true);
	}
	
	if(true){//creates and registers a new alpha service
		insertListener(rw, "gamma", gammaidx, "0.0.0.0", p.start_port + 314, false);
		//insertListener(rw, "alpha", alphaidx, "0.0.0.0", p.start_port + 124, true);
	}
```

Where, the insertListener has the following implementation:

```
	void insertListener(SignalResultWaiter &_rw, const char *_name, IndexT _idx, const char *_addr, int _port, bool _secure){
		concept::AddrInfoSignal *psig(new AddrInfoSignal(_secure? concept::Service::AddSslListener : concept::Service::AddListener, _rw));

		psig->init(_addr, _port, 0, AddrInfo::Inet4, AddrInfo::Stream);
		DynamicPointer<foundation::Signal> dp(psig);
		_rw.prepare();
		concept::m().signalService(_idx, dp);
		int rv;
		switch((rv = _rw.wait())){
			case -2:
				cout<<"["<<_name<<"] No such service"<<endl;
				break;
			case true:
				cout<<"["<<_name<<"] Added listener on port "<<_port<<endl;
				break;
			default:
				cout<<"["<<_name<<"] Failed adding listener on port "<<_port<<" rv = "<<rv<<endl;
		}
	}
```

At the end of main function, after the manager was destroyed we do:

```
	Thread::waitAll();
```

Note that the concept::Manager is declared within a block, and the waitAll() is called outside that block, so that it is called after the manager object is destroyed.

# The concept core library #

While at design level the central concept of the framework is the Pseudo-Active-Object class, at implementation level the central concept is the Manager object which provides the main way to interact with objects and services. As you can see in the above code, the manager is a concept::Manager which is also a foundation::Manager. Lets have a look at concept::Manager (`example/foundation/concept/core/src/manager.cpp`).

The concept::Manager and implicitly the foundation::Manager is the base container for all the services and service like objects (objects that reside within the same MasterService like services).
The concept::Manager is only a thin wrapper for foundation::Manager, as it only initiates some internal services (the ipc) and objects (file manager, some signal executers etc).

Every scheduler, every service and every service like object must be registered to manager before m.start is called.

The manager can be access from anywhere using concept::Manager::the()/foundation::Manager::the() or concept::m()/foundation::m().

The concept\_core library also contains the implementation of the Listener which does the following on its execute method:

```
int Listener::execute(ulong, TimeSpec&){
	Manager &rm = Manager::the();
	Service	&rsrvc = rm.service(*this);
	cassert(this->socketOk());
	if(signaled()){
		{
		Mutex::Locker	lock(mute());
		ulong sm = this->grabSignalMask();
		if(sm & foundation::S_KILL) return BAD;
		}
	}
	uint cnt(10);
	while(cnt--){
		if(state() == 0){
			switch(this->socketAccept(sd)){
				case BAD: return BAD;
				case OK:break;
				case NOK:
					state(1);
					return NOK;
			}
		}
		state(0);
		cassert(sd.ok());
		//TODO: one may do some filtering on sd based on sd.remoteAddress()
		if(pctx.get()){
			rsrvc.insertConnection(rm, sd, pctx.get(), true);
		}else{
			rsrvc.insertConnection(rm, sd);
		}
	}
	return OK;
}
```

Up till now you've learned about the `example/foundation/core` library, which is the basis for the proof of concept multiservice server application. You've learn about the central object, the concept::Manager, you've learned about the base services and the listeners. As you've found out from above, every service has it's own library and folder. Let's take a look at the alpha service which is the most important service/protocol exported by the proof of concept server.

# The alpha service #

As every library in **SolidFrame**, the concept\_alpha library has it own folder. The only public interface of the library is in the `alphaservice.hpp` file, the rest of the code, being private, is within `alpha/src`. In `example/foundation/concept/alpha/alphaservice.hpp` is declared the concept::alpha::Service, which is instantiated in the application's main function and registered into the manager.

A service, once registered, cannot be unregistered. A service can be inactive or active but it cannot be deleted.
As you can see from the code, the alpha service is a concept core service and implements that interface allowing additions of connections and listeners. The only recommended way to directly access a service (I've tried to enforce in the code) is from its PAOs within PAOs execute function. The idea is that the service life will allways exceed its PAOs. See the concept::Listener::execute, above, for an example.

So into an alpha service one can insert connections and listeners. The listeners are of type concept::Listener, while the connections are concept::alpha::Connection(s), private to alpha library.

As the code from the alpha service is pretty simple we shall not delve into it. The only thing I'd like to add, is some words about the destruction of services.

On manager stop this is what is done:
  * The hidden master service (which holds all services and service like objects) is signaled to die and waits until the master service is dead.
  * The master service on its turn, signals all its objects to die, and waits till all objects are released.
  * Then the manager stops all the schedulers.

This is a quite nice approach because as you'll see, the shutdown time is very short, even if lots of connections (and/or other objects) are created.

Next we shall have a look at alpha::Connection (`src/alphaconnection.hpp` and `src/alphaconnection.cpp`).

Because alpha service features, a rather complicated, text protocol (resembling IMAP), the alpha::Connection uses an extended protocol::Reader and protocol::Writer to simplify the asynchronous send, receive/parse operations. The connection also receives different internal signals. I believe the most complicated function from alpha is the alpha::Connection::execute which implements a not so simple state machine:

```
int Connection::execute(ulong _sig, TimeSpec &_tout){
	concept::Manager &rm = concept::Manager::the();
	fdt::requestuidptr->set(this->id(), rm.uid(*this));
	//_tout.add(2400);
	if(_sig & (fdt::TIMEOUT | fdt::ERRDONE)){
		if(state() == ConnectTout){
			state(Connect);
			return fdt::UNREGISTER;
		}else
			idbg("timeout occured - destroy connection");
			return BAD;
	}
	
	if(signaled()){//we've received a signal
		ulong sm(0);
		{
			Mutex::Locker	lock(mute());
			sm = grabSignalMask(0);//grab all bits of the signal mask
			if(sm & fdt::S_KILL) return BAD;
			if(sm & fdt::S_SIG){//we have signals
				grabSignals();//grab them
			}
		}
		if(sm & fdt::S_SIG){//we've grabed signals, execute them
			switch(this->execSignals(*this)){
				case BAD: 
					return BAD;
				case OK: //expected signal received
					_sig |= fdt::OKDONE;
				case NOK://unexpected signal received
					break;
			}
		}
		//now we determine if we return with NOK or we continue
		if(!_sig) return NOK;
	}
	if(socketEvents() & fdt::ERRDONE){
		return BAD;
	}
	int rc;
	switch(state()){
		case Init:
			if(this->socketIsSecure()){
				int rv = this->socketSecureAccept();
				state(Banner);
				return rv;
			}else{
				state(Banner);
			}
		case Banner:{
			concept::Manager	&rm = concept::Manager::the();
			uint32			myport(rm.ipc().basePort());
			ulong			objid(this->id());
			uint32			objuid(rm.uid(*this));
			char			host[SocketAddress::MaxSockHostSz];
			char			port[SocketAddress::MaxSockServSz];
			SocketAddress	addr;
			writer()<<"* Hello from alpha server ("<<myport<<" "<<(uint32)objid<<" "<<objuid<<") [";
			socketLocalAddress(addr);
			addr.name(
				host,
				SocketAddress::MaxSockHostSz,
				port,
				SocketAddress::MaxSockServSz,
				SocketAddress::NumericService | SocketAddress::NumericHost
			);
			writer()<<host<<':'<<port<<" -> ";
			socketRemoteAddress(addr);
			addr.name(
				host,
				SocketAddress::MaxSockHostSz,
				port,
				SocketAddress::MaxSockServSz,
				SocketAddress::NumericService | SocketAddress::NumericHost
			);
			writer()<<host<<':'<<port<<"]"<<'\r'<<'\n';
			writer().push(&Writer::flushAll);
			state(Execute);
			}break;
		case ParsePrepare:
			idbg("PrepareReader");
			prepareReader();
		case Parse:
			//idbg("Parse");
			state(Parse);
			switch((rc = reader().run())){
				case OK: break;
				case NOK:
					if(hasPendingRequests()){
						socketTimeout(_tout, 3000);
					}else{
						_tout.add(2000);
					}
					state(ParseTout);
					return NOK;
				case BAD:
					return BAD;
				case YIELD:
					return OK;
			}
			if(reader().isError()){
				delete pcmd; pcmd = NULL;
				logger.inFlush();
				state(Execute);
				writer().push(Writer::putStatus);
				break;
			}
		case ExecutePrepare:
			logger.inFlush();
			idbg("PrepareExecute");
			pcmd->execute(*this);
			state(Execute);
		case Execute:
			//idbg("Execute");
			switch((rc = writer().run())){
				case NOK:
					if(hasPendingRequests()){
						socketTimeout(_tout, 3000);
						state(ExecuteIOTout);
					}else{
						_tout.add(2000);
						idbg("no pendin io - wait twenty seconds");
						state(ExecuteTout);
					}
					return NOK;
				case OK:
					if(state() != IdleExecute){
						delete pcmd; pcmd = NULL;
						state(ParsePrepare); rc = OK;
					}else{
						state(Parse); rc = OK;
					}
				case YIELD:
					return OK;
				default:
					idbg("rc = "<<rc);
					return rc;
			}
			break;
		case IdleExecute:
			//idbg("IdleExecute");
			if(socketEvents() & fdt::OUTDONE){
				state(Execute);
				return OK;
			}return NOK;
		case Connect:
			/*
			switch(channel().connect(*paddr)){
				case BAD: return BAD;
				case OK:  state(Init);break;
				case NOK: state(ConnectTout); return REGISTER;
			};*/
			break;
		case ConnectTout:
			state(Init);
			//delete(paddr); paddr = NULL;
			break;
		case ParseTout:
			if(socketEvents() & fdt::INDONE){
				state(Parse);
				return OK;
			}
			return NOK;
		case ExecuteIOTout:
			idbg("State: ExecuteTout");
			if(socketEvents() & fdt::OUTDONE){
				state(Execute);
				return OK;
			}
		case ExecuteTout:
			return NOK;
	}
	return OK;
}
```

The idea is:
  * the connection uses a rather complex state machine to asynchronously read and parse the input data using an alpha::Reader, building alpha::Command objects,
  * execute those command objects, which in their turn, instruct the alpha::Writer to asynchronously build the output/command response.

All commands are implemented in src/alphacommands.cpp.

As you can see from alpha code, the connection, not only deals with IO events but also with incoming internal signals (containing: filestreams from FileManager, data from remote hosts in case of remotelist or fetch commands). Interesting is the way the connection deals with the incoming internal signals, the way it forwards them to the current alpha::Command object:

```
typedef DynamicExecuter<void, Connection>	DynamicExecuterT;
//...
/*static*/ void Connection::dynamicRegister(){
	DynamicExecuterT::registerDynamic<IStreamSignal, Connection>();
	DynamicExecuterT::registerDynamic<OStreamSignal, Connection>();
	DynamicExecuterT::registerDynamic<IOStreamSignal, Connection>();
	DynamicExecuterT::registerDynamic<StreamErrorSignal, Connection>();
	DynamicExecuterT::registerDynamic<RemoteListSignal, Connection>();
	DynamicExecuterT::registerDynamic<FetchSlaveSignal, Connection>();
	DynamicExecuterT::registerDynamic<SendStringSignal, Connection>();
	DynamicExecuterT::registerDynamic<SendStreamSignal, Connection>();
}
```

```
//receiving an istream
int Connection::dynamicReceive(IStreamSignal &_rsig){
	idbg("");
	if(_rsig.requid.first && _rsig.requid.first != reqid) return NOK;
	idbg("");
	newRequestId();//prevent multiple responses with the same id
	if(pcmd){
		switch(pcmd->receiveIStream(_rsig.sptr, _rsig.fileuid, 0, ObjectUidTp(), NULL)){
			case BAD:
				idbg("");
				break;
			case OK:
				idbg("");
				if(state() == ParseTout){
					state(Parse);
				}
				if(state() == ExecuteTout){
					state(Execute);
				}
				break;
			case NOK:
				idbg("");
				state(IdleExecute);
				break;
		}
	}
	return NOK;
}
```


Notable alpha commands are: concept::alpha::RemoteList and concept::alpha::Fetch - see their implementation in `solidframe/example/foundation/concept/alpha/src/alphacommands.cpp`.

Alpha protocol is a concept protocol designed as a proof of concept (showing the way the **SolidFrame** foundation libraries should/may be used). The implementation though, is both not perfect and incomplete. It serves as a testing platform for new features and for profiling in combination with some clients (`example/client/alpha`**``).**

# Closing #

This presentation is not complete, there is lot of code and I believe that sample code "speaks better". So don't hesitate browsing the sample code and the framework code.

I'm sure that from this presentation you haven't learned to program with **SolidFrame**, but I hope you've at least started to understand the design and what it offers. I also hope you've liked the design enough to have a better in-depth look at the code and maybe, start your own project to use **SolidFrame**.