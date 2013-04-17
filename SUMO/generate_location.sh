#!/bin/bash
for i in `seq -w 1 10`; do 
	cd run$i/; 
	sudo perl ../build_location.pl activity.tcl netstate6_4.xml; 
	cd ..; 
done
