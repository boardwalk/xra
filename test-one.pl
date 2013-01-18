#!/usr/bin/perl
use strict;
use warnings;

my $filePath = shift or die "Usage: $0 <path>";

my %opt;
$opt{expect} = 'success';

open(my $fh, $filePath) or die "Failed to open $filePath: $!";
while(<$fh>) {
  $opt{$1} = $2 if(/##\s*(\S+)\s*=\s*(\S+)\s*$/);
}
close($fh);

my @args;
push @args, "src/xra";
push @args, $opt{args} if $opt{args};
push @args, $filePath;

my $code = system(@args);
die "Failed to run script: $filePath" if(($code == 0) != ($opt{expect} eq 'success'));
