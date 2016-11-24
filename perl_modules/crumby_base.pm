#!/usr/bin/perl -w
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Copyright 2015 icke2063 <icke2063@gmail.com>

use strict;
use warnings;
use utf8; # this script is written in utf8


use Switch;
use MBclient;

package crumby::crumby;

sub check_forValidSlave
{
    my $mb_ctx = shift;
    my $host_addr = shift;
    my $host_port = shift;
    my $unit_id = shift;

    # define server target
    $mb_ctx->host($host_addr);
    $mb_ctx->port($host_port);
    $mb_ctx->unit_id(255);

    # read 16 bits register from addr unit_id
    my $words = $mb_ctx->read_input_registers($unit_id, 1);

    if( @$words[0] > 0 )
    {
	return 1;
    }  
    else
    {
	return 0;
    }
}

sub get_SlaveID
{
    my $mb_ctx = shift;
    my $host_addr = shift;
    my $host_port = shift;
    my $unit_id = shift;

    # define server target
    $mb_ctx->host($host_addr);
    $mb_ctx->port($host_port);
    $mb_ctx->unit_id(255);

    # read 16 bits register from addr unit_id
    my $words = $mb_ctx->read_input_registers($unit_id, 1);

#    printf "unitID[%i]:0x%x\n", $unit_id, @$words[0];

    return @$words[0];
}

1;