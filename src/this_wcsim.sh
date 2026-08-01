# Sets necessary environment variables to use this installation of WCSim in ROOT
# Also set PATH and LD_LIBRARY_PATH for convenience

export WCSIM_BUILD_DIR=/eos/home-j/jrimmer/sim_work_dir/WCSim/src/
export ROOT_INCLUDE_PATH=/eos/home-j/jrimmer/sim_work_dir/WCSim/src/include/WCSim:$ROOT_INCLUDE_PATH
export PATH=/eos/home-j/jrimmer/sim_work_dir/WCSim/src/bin:$PATH
export LD_LIBRARY_PATH=/eos/home-j/jrimmer/sim_work_dir/WCSim/src/lib:$LD_LIBRARY_PATH
export WCSIM_SOURCE_DIR=/eos/home-j/jrimmer/sim_work_dir/WCSim/
