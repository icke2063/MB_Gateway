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

package crumby::ioboard;

sub MBread_IOBOARD_pin_count
{
    my $mb_ctx = shift;
    my $host_addr = shift;
    my $host_port = shift;
    my $unit_id = shift;

    # define server target
    $mb_ctx->host($host_addr);
    $mb_ctx->port($host_port);
    $mb_ctx->unit_id($unit_id);

    # read 16 bits register from addr 8 - Pin Count
    my $words = $mb_ctx->read_holding_registers(8, 1);

    if ( (defined $words) ) 
    {
        return @$words[0] >> 8;
    }
    else
    {
	return undef;
    }

    return -1;
}


sub MBread_IOBOARD_pin_function
{
    my $mb_ctx = shift;
    my $host_addr = shift;
    my $host_port = shift;
    my $unit_id = shift;
    my $virt_pin = shift;

    # define server target
    $mb_ctx->host($host_addr);
    $mb_ctx->port($host_port);
    $mb_ctx->unit_id($unit_id);

    my $w_func = -1;
    # read 16 bits register from addr 8 - Pin Count
    my $mb_addr = (0x100/2) + (0x10/2) + $virt_pin;
    $w_func = $mb_ctx->read_holding_registers($mb_addr, 1);

    if ( (defined $w_func) ) 
    {
        return @$w_func[0];
    }
    else
    {
	return undef;
    }

    return -1;
}


#####
# Get state of virtual IO port of I2C_IO_Board via crumby MB_Gateway
#
# parameter:
# - mb_ctx		MBClient handle
# - host address:       TCP/IP Address of crumby MB server
# - unit_id:            slave ID of IO_Board
# - virt_port:          port number of virtual port of IO Board [0..1]
#
# return:
# -  port value :       valid
# - -1  :               invalid
# -

sub MBread_IOBOARD_vioport
{

    my $mb_ctx = shift;
    my $host_addr = shift;
    my $host_port = shift;
    my $unit_id = shift;
    my $virt_port = shift;


    # define server target
    $mb_ctx->host($host_addr);
    $mb_ctx->unit_id($unit_id);

#naapp    printf "[vioport]port: %i\n", $virt_port;

    # read 16 bits register from ad 0 to 9
    my $words = $mb_ctx->read_holding_registers(10 + $virt_port, 1);

    if( !(defined $words) ) {
        return undef;
    }

    if(@$words > 0){
        my $port = @$words[0] >> 8;
        return $port;
    }

return -1;

}

#####
# Get state of virtual IO pin of I2C_IO_Board via crumby MB_Gateway
#
# parameter:
# - virt_pin:           pin number of virtual pin of IO Board [0..7]
# - MBread_VIOPORT

# return:
# -  0  :       pin off
# -  1  :       pin on
# - -1  :
# -

sub MBread_IOBOARD_viopin
{

    my $port = MBread_IOBOARD_vioport(@_);

    my $mb_ctx = shift;
    my $host_addr = shift;
    my $host_port = shift;
    my $unit_id = shift;
    my $virt_port = shift;
    my $virt_pin = shift;


#    printf "[viopin]port: 0x%x\n", $port;
#    printf "[viopin]pin: %i\n", $virt_pin;


    if( !(defined $port) or $port < 0 ) 
    {
        return -1;
    }

        #check value
        if( $port & (1<<($virt_pin%8)) ){
            #on
            return 1;
        } else {
            #off            
            return 0;
        }

return -1;

}

1;
