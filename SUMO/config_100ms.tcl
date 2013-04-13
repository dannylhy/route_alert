# set number of nodes
set opt(nn) 221.0

# set activity file
set opt(af) $opt(config-path)
append opt(af) /activity_100ms.tcl

# set mobility file
set opt(mf) $opt(config-path)
append opt(mf) /mobility_100ms.tcl

# set start/stop time
set opt(start) 0.0
set opt(stop) 197.39999389648438

# set floor size
set opt(x) 759
set opt(y) 449
set opt(min-x) -9
set opt(min-y) 1
