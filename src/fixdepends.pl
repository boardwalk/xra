#!/usr/bin/perl
my $file = shift;

my @deps;
while(<>) {
  /:/g;
  push @deps, $& while(/[^\s\\]+/g);
}

@deps = map {
  if(index($_, '/') < 0) { "obj/$_.md5" } else { $_ }
} @deps;

print "obj/$file.o obj/$file.d: ", join(' ', @deps), "\n";
