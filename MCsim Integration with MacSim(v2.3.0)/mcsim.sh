git clone https://github.com/uwuser/MCsim.git ./src/MCsim
patch -p0 -i patch/mcsim_make.patch
cd src/MCsim
patch < ../../patch/mcsim_src.patch
