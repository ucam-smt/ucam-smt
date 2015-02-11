#!/usr/bin/perl -w

#################################################
### prepares a shallow grammar
### input: the output of ruleXtract retrieval
### output: the shallow grammar
#################################################

use strict;

while (<>) {
    chomp;
    my @parts = split(/ /, $_, -1);
    if ($parts[2] eq "") {
		$parts[2] = "<oov>";
    }
    $parts[0] =~ s/0/V/;
    $parts[0] =~ s/-1/X/;
    $parts[0] =~ s/-4/S/;
    $parts[1] =~ s/-1/V/g;
    $parts[1] =~ s/-2/V/g;
    $parts[1] =~ s/-3/W/g;
    $parts[1] =~ s/-4/S/g;
    $parts[1] =~ s/S_V/S_X/;
    $parts[2] =~ s/^0$/<dr>/;
    $parts[2] =~ s/-1/V/g;
    $parts[2] =~ s/-2/V/g;
    $parts[2] =~ s/-3/W/g;
    $parts[2] =~ s/-4/S/g;
    $parts[2] =~ s/S_V/S_X/;
    $parts[2] =~ s/^1$/<s>_<s>_<s>/;
    $parts[2] =~ s/^2$/<\/s>/;
    my $rule = join(" ", @parts);
    if ($rule =~ /V.*W.*W.*V/) {
		$rule =~ s/V/T/g;
		$rule =~ s/W/V/g;
		$rule =~ s/T/W/g;
    }
    if ($rule =~ /^V.*V/) {
		$rule =~ s/^V/X/;
    }
    if ($rule =~ /^W.*V/) {
		$rule =~ s/^W/X/;
    }
    if ($rule =~ /^S.*V/) { # S V V becomes S X X
		$rule =~ s/V/X/g;
    }
    print "$rule\n";
}
