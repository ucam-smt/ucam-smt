#!/usr/bin/perl -w
#$ -S /usr/bin/perl

use Getopt::Long;

my $usage="Usage: $0 

-?           help\n";

my $h = undef;

GetOptions("h!" => \$h);

die $usage if ($h);

while (<STDIN>) {
  chomp;
  if (m/BLEU:/) {
    my @tokens = split(" ", $_);

    my $p1 = $tokens[1];
    my $p2 = $tokens[2];
    my $p3 = $tokens[3];
    my $p4 = $tokens[4];

    my $r43 = $p4 / $p3;
    my $r32 = $p3 / $p2;
    my $r21 = $p2 / $p1;

    my $p = sprintf("%.4f", $p1);
    my $r = sprintf("%.4f", ($r43 + $r32 + $r21) / 3);

    printf("1g\t%.4f\n", $p1);
    printf("2g\t%.4f\t%.4f\n", $p2, $r21);
    printf("3g\t%.4f\t%.4f\n", $p3, $r32);
    printf("4g\t%.4f\t%.4f\n", $p4, $r43);

    print "\np=$p\nr=$r\n";

    last;
  }
}
