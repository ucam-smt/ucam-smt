#!/usr/bin/perl -w

#################################################
### prepares a shallow grammar readable by the
### HiFST decoder
### input: a shallow grammar
### output: a shallow grammar readable by HiFST
#################################################

use strict;

my $lineout;
my $negativeFeature;
while (<>) {
    chomp;
    if (/nan/) {
		print STDERR "WARNING: nan in rule file in line: $_\n";
		next;
    }
    if (/^S X X/) {
		next;
    }
    s/W/V1/g;
    my @parts = split(/\s+/);
    if (@parts == 3) {
		$_ .= " 0\@1";
		@parts = split(/\s+/);
    }
    if ($parts[2] eq "<dr>") {
		$parts[0] = "D";
    } elsif ($parts[0] eq "S" && $parts[1] eq "S_X" && $parts[2] eq "S_X") {
		$parts[1] = "S_D_X";
		$parts[2] = "S_D_X";
    } elsif ($parts[2] eq "<s>_<s>_<s>") {
		$parts[0] = "S";
		$parts[2] = "1";
    }
    $lineout = "$parts[0] $parts[1] $parts[2]";
    for (my $i = 3; $i < @parts; $i++) {
		if ($parts[$i] =~ /^(.*)(\@\d+)$/) {
			if ($1 == 0) {
				$negativeFeature = 0;
			} elsif ($parts[1] eq "S_D_X") {
				$negativeFeature = -2 * $1;
			} else {
				$negativeFeature = -1 * $1;
			}
			$lineout .= " " . $negativeFeature . $2;
		} else {
			print STDERR "ERROR: wrong format for rule: $_\n";
		}
    }
    if ($parts[1] eq "S_D_X") {
		$lineout .= "\nS S_X S_X";
		for (my $i = 3; $i < @parts; $i++) {
			if ($parts[$i] =~ /^(.*)(\@\d+)$/) {
				$negativeFeature = -1 * $1;
				$lineout .= " " . $negativeFeature . $2;
			} else {
				print STDERR "ERROR: wrong format for rule: $_\n";
			}
		}
    }
    print "$lineout\n";
}
