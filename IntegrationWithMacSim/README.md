# MCsim can be used as part of the full system simulation. We hooked up MCsim with MacSim.

G++/GCC toolchain verseion is tested with 4.8. Make sure the permission is given in the directory: chmod u+x ./build.py 

To run in full system simulation mode, follow:

```
 $ cd IntegrationWithMacSim
 $ ./build --clean
 $ cd bin
 $ ./build --mcsim
 
```

 However, you need to have params.in and trace_file_list files in the same directory. Note that the memory controller of MacSim will be replaced with the controller modeled in MCsim. To configure the MCsim controller, direct to: 

```
 $ cd IntegrationWithMacSim/src/
 
```

and introduce the desired controller in dram.cc. 

== Different version of macsim ==
  Optimized version (default)
  Debug version
    ./build.py -d
  Gprof version
    ./build.py -p
