#!/bin/sh
duarouterD -c grid6_4.duarcfg

sumoD -c grid6_4.sumocfg

java -jar /home/mikel/school/GT/sumo-0.16.0/tools/traceExporter/traceExporter.jar ns2 -n ../grid6_4.net.xml -t netstate6_4.xml -a activity.tcl -m mobility.tcl -c config.tcl -p 1 -b 0 -e 250

grep Amb activity.tcl > README
grep F04 grid6_4.flo.xml >> README
