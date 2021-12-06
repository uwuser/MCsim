# MCsim: An Extensible DRAM Memory Controller Simulator

MCsim is a cycle-accurate DRAM memory controller simulation platform designed in C++ that takes benefit of the extensibility of classes. MCsim provides an interface to connect with external hardware simulators such as CPU and memory system simulators. With virtual functions, MCsim provides a simple interface for the designer to develop a scheduling policy effectively without re-implementing the other parts of the hardware. 

If you use this simulator in your work, please consider cite:


Mirosanlou, Reza., Guo, Danlu., Hassan, Mohamed., Pellizzoni, Rodolfo. "MCsim: An extensible dram memory controller simulator," in IEEE Computer Architecture Letters. [MCsim](https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=9137661).


MCsim supports many real-time and conventional scheduling policies in the controller designs such as:

  * [REQBundle](https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=7939044)
  * [CMDBundle](https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=7383564) 
  * [ORP](https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=6728891) 
  * [MEDUSA](https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=7272690) 
  * [RTMem](https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=6932585) 
  * [ROC](https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=6932587)
  * [RankReOrder](https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=7557864)
  * [MCMC](https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=6910550) 
  * [AMC](https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=5401062) 
  * [BLISS](https://users.ece.cmu.edu/~omutlu/pub/bliss-memory-scheduler_iccd14.pdf) 
  * [DCmc](https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=7010488)
  * [FCFS](http://www.dropwizard.io/1.0.2/docs/)
  * [FR-FCFS](https://scholarship.rice.edu/bitstream/handle/1911/20279/Rix2000Jun5MemoryAcce.PDF?sequence=1) 
  * [MAG]() 
  * [PAR-BS](https://users.ece.cmu.edu/~omutlu/pub/parbs_isca08.pdf) 
  * [PMC](https://caesr.uwaterloo.ca/papers/hassan_15_pmc.pdf)

# Usage

MCsim reads the instruction traces from a file, and simulates a simplified model of a "core" that generates memory requests to the DRAM subsystem. Each line in the trace file represents a memory request, and can have the following format:

```
<addr> <read/write> <cpu-inst>
 ```
The first token represents the address of the request. The second token accounts for the type of access and the last token represents the number of CPU instructions before the memory request

# Building and Running MCsim - Standalone

MCsim requires a C++11 compiler (e.g., clang++, g++-5). To build an optimized MCsim simply follow:


```
 $ cd MCsim/src
 $ make 
 $ ./MCsim -n <# of requestors> -s system/<system.ini> -G <DeviceType> -D <DeviceSpeed> -S <DeviceSize> -R <# of ranks> -t Mem_Trace/<trace.trc>  -c <cycles>  
```
In order to enable the DEBUG mode, simply uncomment DDEBUG_ENABLED flag in the makefile. There exists an option to run the simulation as follows.

```
-t memory trace list that assigned to each requestor
-s memory controller system employed
-C number of channels running in parallel
-R number of ranks in a memory channel
-G memory device generation: DDR3
-D memory device speed: 1600H
-S memory device size: 2Gb_x8
-n number of requestors target to the memory system
-c number of cycles for simulation

```

# Building and Running MCsim - full-system simulation

MCsim can be integrated into full-system simulators. We provide a library (libmcsim.so) in order to prepare the interfaces for the CPU simulators. After building the MCsim, follow: 

```
 $ make libmcsim.so
```
 
# Memory Controller Configurations

A specific memory controller is built by a configuration file (system.ini}) to define the structure of the queues as well as the operation of each hardware block. As an example, we show the configuration for the ORP controller, which requires per requestor buffers and employs "DIRECT" request arbitration, "OPEN" command generation and a specific "ORP" command scheduling policy.

```
RequestBuffer=0000    // request queue per level
ReqPerREQ=1           // request queue per requestor
WriteBuffer=0         // dedicated queue for write requests
CommandBuffer=0000    // command queue per level
CmdPerREQ=1           // command queue per requestor
// scheduler Based on Keys
RequestScheduler='DIRECT'// employ "DIRECT" request scheduler
CommandGenerator='OPEN'  // employ "OPEN" command generator
CommandScheduler='ORP'   // employ "ORP" command scheduler

```

The queue configuration can be associated based on the memory hierarchy (channel, rank, bankgroup, bank, subarray) OR it can be per requestor such that each master entity in the system has its own separate queue structure. This configuration is similar between command and request queue structure. 

# Develop a New MC

To develop a new memory controller in MCsim, you need to write your implementation in the system directory. This depends on which level you want to implement your code (request scheduler, command scheduler, or/and command generator). Then, you need to specify your system.ini file, which describes the structure of queues/address mapping and etc.

# MCsim Output Modes 

In order to track the command trace and request trace that is scheduled from any MC, there exist the following flags in the makefile.

*  Printing the command trace:

       Enable -DCMD_TRACE_ENABLED
       
*  Printing the request trace:

       Enable -DREQ_TRACE_ENABLED

# Address Mapping Configuration

Since the sample memory traces are generated without considering allocating to individual rank and bank, the user is supposed to manually reconfigure the address location in the AddressMapping class. The configuration is done through MAKEFILE compile options as follows:

* Interleaved controllers (AMC, RTMem, PMC)
    Normal translation based on the defined bits order in the system.ini
    
        Disable all privatization options

* Single rank bank privatized controllers (ORP, DCmc, MAG, PipeCAS, ReOrder)
    After normal translation, change the bank to match requestor ID. 
    This ensures that each requestor is isolated
    
        Enable -DSINGLE_RANK_BANK_PRIVATIZATION

* Multiple Rank Bank Privatized Controllers (ROC, MCMC)
    After normal translation, change the rank and bank accordingly based on the requestor ID.
    This ensures that each requestor is allocated to isolated rank and bank
    
        Enable -DMULTI_RANK_BANK_PRIVATIZATION

# Core Configuration

MCsim supports criticality for cores and also is able to simulate in-order and out-of-order execution. The core(requestor) criticality and execution modes also require manual assignment in the main.cpp and memorycontroller class, as well as request size. For each requestor, there are three parameters.

```
bool inOrder = true;        // If the requestor is executing memory request in order
bool isHRT = true;          // If the requestor is more critical than the others
int requestSize = 64;       // Size of the memory request

```

# Refresh Mechanisms

MCsim is capable of supporting different and common refresh mechanisms. For example, MCsim provides the following schemes:  

*  Distributed refreshes: At any interval tREFI, corresponding banks from each rank will be refreshed. This can be done by chosing refresh mechanism in RefreshMachine.h file.

       refresh_mechanism = "distributed";
       
*  Per-bank refreshes: At any partial interval tREFIpb, corresponding banks will be refreshed based on the arbitration mechanism (such as round-robin).

       refresh_mechanism = "per-bank";

In order to disable the refreshes, "none" can be chosed as refresh_mechanism.

# Code Structure

```
├── dram               			  # Configs of various protocols that describe timing constraints.
├── general scheduler                     # General arbitration mechanisms
├── src  			  	  # MCsim source files    
	├── Makefile 		 	  # Makefile
	├── Mem_Trace                	  # Test traces 
├── system                  	   	  # Detailed implementation of each MC along with .ini configuration file
└── README.md

    AddressMapping.cpp: Responsible for address translation scheme determined by system.ini configuration file. 
    CommangGenerator.cpp: Generating the DDR command according to the type of the incoming request.  
    CommandQueue.cpp: Maintains command queue structures determined by parameters in system.ini.
    RequestQueue.cpp: Maintains request queue structures determined by parameters in system.ini.
    MemorySystem.cpp: Responsible for communicating with the memory device
    RequestScheduler.cpp: Imepelements the arbitration in request level.  
    Requestor.cpp:  Implements and handle the requestor behaviours.   
    MemoryController: Implements the top module of the simulator, handling primary functions.
    main.cc: Handles the main program loop that reads in simulation options, DRAM device configurations and tick the cycle. Determine the requestor criticality and execution order.
    CommandScheduler.cpp	:  Imepelements the arbitration in command level.  

  
```

# Simulator Output

Upon finishing a trace file from core under analysis (REQ0), the simulation will end, and the stats will be printed. This includes the worst-case latency of the READ/WRITE (open/close) requests, as well as the simulation time and bandwidth. In order to track the operation of the controller at each clock cycle, you may enable the debug flags. The debug format is consists of two formats; one for the requests and the other one for commands. Notice that the stats assume that the cores are in order. In the case of using OoO cores, the stats must be modified according to the WC definitions.  


