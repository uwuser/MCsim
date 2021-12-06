# MCsim: An Extensible DRAM Memory Controller Simulator

MCsim is a cycle-accurate DRAM memory controller simulation platform designed in C++ that takes benefit of extensibility of classes. MCsim provides interface to connect with external hardware simulators such as CPU and memory system simulators. With virtual functions, MCsim provides a simple interface for designer to develop scheduling policy effectively without re-implementing the other parts of the hardware. MCsim suports many real-time and conventional scheduling policies in the controller designs such as:

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
The first token represents the address of the request. The second token accounts for the type of the access and the last token represnts the number of CPU instructions before the memory request

# Building and Running MCsim

MCsim requires a C++11 compiler (e.g., clang++, g++-5). To build an optimized MCsim simply follow:


```
 $ cd MCsim
 $ make 
 $ ./MCsim -t Mem_Trace/<trace.trc> -s system/<system.ini> -c <cycles> -n <# of requestors> -R <# of ranks> -D <device>
```
In order to enable the DEBUG mode, simply un comment DDEBUG_ENABLED flag in the makefile. There exists option to run the simulation as follows.

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
# MCsim Debug Modes 

In order to track the command trace and request trace that are scheduled from any MC, there exists the following flags in the makefile. 

*  Printing the command trace:

       Enable -DCMD_TRACE_ENABLED
       
*  Printing the request trace:

       Enable -DREQ_TRACE_ENABLED
# Develop a New MC
To develop a new memory controller in MCsim, you need to write your imeplemetation in the system directory. This depend on which level you want to implement your code (requestScheduler, commandScheduler, or/and commandGenerator). Then, you need to specify your system.ini file which describe the structure of queues/address mapping and etc.


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
MCsim supports criticality for cores and also is able to simulate in-order and out-of-order execution. The core(requestor) criticality and execution modes also require manual assignment in the main.cpp, as well as request size. For each requestor, there are three parameters
```
bool inOrder = true;        // If the requestor is executing memory request in order
bool isHRT = true;          // If the requestor is more critical than the others
int requestSize = 64;       // Size of the memory request

```

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

Upon finishing a trace file from core under analysis (REQ0), the simulation will end and the stats will be printed. This includes the worst case latency of the READ/WRITE (open/close) requests as well as the simulation time and bandwidth. In order to track the operation of the controller at each clock cycle, you may enable the debug flags. The debug format is consist of two format; one for the requests, and the other one for commands. Notice that, the stats assume that the cores are in order. In case of using OoO cores, the stats must be modified according to the WC definitions.  


# Validation

In orther to provide a fair comparison among MCsim, ramulator, and DRAMsim2 when evaluating FR-FCFS scheduler, we considered the following configurations. The address mapping of all simulators is RowBnkCol. In order to achieve this in the dram mode simulation of ramulator we employed the MCsim_mapping.map as follows:

```
# Standard         DDR3
# Number of bits     41
# Channel             0
# Rank                0
# Bank                3
# Row                28 
# Column             10

Co  9:0 =  9:0
Ba  2:0 = 12:10
Ro 27:0 = 40:13

Notice that, if the mapping flag is used in ramulator, the simulation time will be delayed significantly. In order to provide a fair comparison, we have implemented the address mapping in Memory.h where address translation is handled. 

```
Notice that, for ramulator, all the print status activites must be disabled as they impose delay at run time. Regarding the DRAMsim2, we have implemented an extra scheme in the AddressMapping.cpp. For all simulators, we employed 32 entry queues for read and write request. For the purpose of verification, we have disabled the refresh mechanims for all the simulators. Since there is no DDR3 1600K device in the dramsim2, we have generated the correspoding .ini file for this device according to the timing constraints from JEDEC. Notice that in order to use the FR-FCFS or other mechanisms that require re-ordering in the request level, the in-order flag in main.cpp of MCsim should be set to false. In addition, currently the status counters are for RT MCs as they are concerecd with the worst case times.

