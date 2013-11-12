#!/usr/bin/env perl

use strict;
use warnings;

my @CT = ();

while (<>) {
  chomp;
  next if /^\s*#/;
  next if /^\s*$/;
  my @f = split /\s+/, $_, 6;
  die "Malformed line" unless @f == 6;
  push @CT, parse_rule(@f);
}

sub parse_rule {
  my @f = @_;
}

# vim:ts=2:sw=2:sts=2:et:ft=perl

