#!/bin/bash
for i in `seq -w 1 10`; do 
	cd run$i/; 
	sudo perl ../build_location.pl activity.tcl netstate6_4.xml; 
	cat mobility.tcl > mobility_processed.tcl
	echo "\$node_(251)   set X_ 754.95 # SUMO-ID: Destination Node" >> mobility_processed.tcl
  	echo "\$node_(251)   set Y_ 424.503" >> mobility_processed.tcl

  	echo "\$node_(251)   set Z_ 0.0" >> mobility_processed.tcl

	cd ..; 
done
