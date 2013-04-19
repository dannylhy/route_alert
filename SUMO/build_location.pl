#!/usr/bin/perl

# build_location.pl
# Builds the location.cc and location.h files from the input SUMO files
# These 2 files need to go into the /src/buffer-and-switch/model directory

use Switch;
use Data::Dumper;
use XML::Parser;
use Clone qw(clone);
use Math::Round;

if (@ARGV != 2) {
    print "Usage: perl build_location.pl activity.tcl netstate.xml\n";
    exit
}

my $activity_tcl = $ARGV[0];
my $netstate_xml = $ARGV[1];

open TCL_IN, $activity_tcl;
open CC_OUT, ">location.cc";
open H_OUT, ">location.h";

my %sumo_id_mapping = ();

foreach my $line (<TCL_IN>) {
    if (($id, $sumo) = $line =~ m/\"\$g\((\d*)\).*SUMO-ID:\s(.*)$/) {
	$sumo_id_mapping{$sumo} = $id;
    }
}

#print Dumper \%sumo_id_mapping;

my $g_time = 0.0;
my $g_road = undef;
my %g_vehicles = ();

my %table = ();

sub handle_start {
    my $expat = shift;
    my $element = shift;

    if ($element eq "timestep") {
	my $attr = shift;
	my $val = shift;
	#print "timestep $attr = $val\n";
	$g_time = nearest(0.1, $val);
    }
    elsif ($element eq "edge") {
	my $attr = shift;
	my $val = shift;
	#print "edge $attr = $val\n";
	$g_road = $val;
    }
    elsif ($element eq "vehicle") {
	my %veh_stats = ();

	my $attr = shift;
	my $val = shift;
	#print "v1 $attr = $val\n";
	$veh_stats{$attr} = $val;

	my $attr = shift;
	my $val = shift;
	#print "v2 $attr = $val\n";
	$veh_stats{$attr} = $val;

	my $attr = shift;
	my $val = shift;
	#print "v3 $attr = $val\n";
	$veh_stats{$attr} = $val;

	$veh_stats{'road'} = $g_road;

	#print Dumper \%veh_stats;

	# add to global
	if (exists($sumo_id_mapping{$veh_stats{'id'}})) {
	    $g_vehicles{$sumo_id_mapping{$veh_stats{'id'}}} = \%veh_stats;
	} else {
	    print "Mapping for $veh_stats{'id'} doesn't exist!\n";
	}
    }
}

sub handle_end {
    my $expat = shift;
    my $element = shift;

    if ($element eq "timestep") {
	#print Dumper \%g_vehicles;
	$table{$g_time} = clone \%g_vehicles;
	%g_vehicles = ();
    }
    elsif ($element eq "edge") {

    }
}

# create object
$p1 = new XML::Parser(Handlers => {Start => \&handle_start,
				   End   => \&handle_end});
$p1->parsefile($netstate_xml);

#print Dumper \%table;

my $first = 0;

# Build the h file
print H_OUT "#include \"ns3/simulator.h\"\n";
print H_OUT "using namespace std;\n";
print H_OUT "namespace ns3{\nnamespace bs{\n";
print H_OUT "string GetRoad(uint32_t in_id);\n";
print H_OUT "}\n}\n";

# Build the cc file
print CC_OUT "#include \"ns3/simulator.h\"\n";
print CC_OUT "using namespace std;\n";
print CC_OUT "namespace ns3{\nnamespace bs{\n";
print CC_OUT "string GetRoad(uint32_t in_id) {\n";

print CC_OUT "Time now_time = ns3::Simulator::Now();\n";
print CC_OUT "double in_time = (double) (((int) ((now_time.GetMilliSeconds() / 100)+0.5)) / 10.0f);\n";
print CC_OUT "if (in_id == 251)\n  return \"L1206\";\n";

foreach my $time (sort { $a <=> $b } keys %table) {
    my $vehicles = $table{$time};

    if ($first) {
	printf CC_OUT "else if (in_time <= %.1f) {\n", $time;
    }
    else {
	printf CC_OUT "if (in_time <= %.1f) {\n", $time;
	$first = 1;
    }

    if (!%{$vehicles}) {
	print CC_OUT "\treturn \"\";\n";
	print CC_OUT "}\n";
	next;
    }

    print CC_OUT "\tswitch(in_id) {\n";
    foreach my $v_id (sort { $a <=> $b } keys %{$vehicles}) {
	my $v_stats = $vehicles->{$v_id};

	print CC_OUT "\t\tcase $v_id :\treturn \"$v_stats->{'road'}\";\n";
    }
    print CC_OUT "\t\tdefault :\treturn \"\";\n";
    print CC_OUT "\t};\n";
    print CC_OUT "}\n";
}
print CC_OUT "return \"\";\n";
print CC_OUT "};\n";
print CC_OUT "}\n}\n";
