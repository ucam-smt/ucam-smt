#!/usr/bin/perl -w

#################################################
### converts from sparse to nonsparse format
### input: a grammar in sparse format
### output: a grammar in nonsparse format
#################################################

use strict;

if (@ARGV != 1) {
    print STDERR "Args: <number of features in the rule file (don't count the lm)> (input is stdin, output is stdout)\n";
    exit(1);
}

my $nbfeatures = shift @ARGV;

while (<>) {
    chomp;
    my @parts = split(/\s+/);
    if (@parts < 4) {
		print STDERR "Wrong format for rule, should have at least lhs, src, trg and one feature: $_\n";
		exit(1);
    }
    my @features = (0) x $nbfeatures;
    for (my $i = 3; $i < @parts; $i++) {
		if ($parts[$i] =~ /(.*)\@(\d+)/) {
			my $index = $2 - 1;
			$features[$index] = $1;
		} else {
			print STDERR "Wrong format for feature: $parts[$i] for rule: $_\n";
			exit(1);
		}
    }
    print "$parts[0] $parts[1] $parts[2] " . join(" ", @features) . "\n";
}
