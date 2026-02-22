sysctl -n hw.logicalcpu

pio run -e mbot2_hybrid -j 10 # Use all available CPU cores for compilation
pio run -e mbot2_hybrid -t uploadfs
pio run -e mbot2_hybrid -t upload

# pump pio run -e mbot2_hybrid -j 24 # Use all available CPU cores for compilation with pump. Core * 1,2