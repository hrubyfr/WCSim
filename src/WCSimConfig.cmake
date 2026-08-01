# WCSimConfig.cmake

get_filename_component( WCSim_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH )

include( CMakeFindDependencyMacro )
find_dependency( Geant4 REQUIRED )
find_dependency( ROOT REQUIRED )

include("${WCSim_CMAKE_DIR}/WCSim_Library_Targets.cmake")

set (WCSIM_CHECK_GEOMETRY_OVERLAPS $<IF:$<BOOL:OFF>,true,false>)
set (WCSIM_CHECK_GEOMETRY_PMT_OVERLAPS $<IF:$<BOOL:OFF>,true,false>)
set (WCSIM_INCLUDE_DIRS /eos/home-j/jrimmer/sim_work_dir/WCSim/src/include/WCSim)

find_library(WCSimRoot_LIBRARIES REQUIRED
        NAMES WCSimRoot
        HINTS /eos/home-j/jrimmer/sim_work_dir/WCSim/src/lib)

find_library(WCSimCore_LIBRARIES REQUIRED
        NAMES WCSimCore
        HINTS /eos/home-j/jrimmer/sim_work_dir/WCSim/src/lib)

list(APPEND WCSim_LIBRARIES
        ${WCSimCore_LIBRARIES}
        ${WCSimRoot_LIBRARIES})
