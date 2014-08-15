#!/usr/bin/perl -w

##############################################
### makes a Hadoop local configuration file
### applies to mapred-site.xml, hdfs-site.xml,
### and core-site.xml
### input: standard input
### output: standard output
##############################################

use strict;

my $config = 0;

while (<>) {
    chomp;
    if (/<configuration>/) {
	$config = 1;
	print "$_\n";
    } elsif (/<\/configuration>/) {
	$config = 2;
	print "$_\n";
    } elsif ($config == 0 || $config == 2) {
	print "$_\n";
    }
}
