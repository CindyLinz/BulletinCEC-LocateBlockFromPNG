#!/usr/bin/perl

use strict;
use warnings;

use List::Util qw(reduce max min);

my @field = qw(
    號次
    相片
    姓名
    出生年月日
    性別
    出生地
    推薦之政黨
    學歷
    經歷
    政見
);
my $field_num = @field;

my $input = do {
    local $/;
    <STDIN>;
};

my @block;
while( $input =~ /(\d+)\D+(\d+)\D+(\d+)\D+(\d+)/g ) {
    push @block, [$1, $2, $3, $4];
}
@block = sort { $a->[1] <=> $b->[1] } @block;

my @row;
while( @block ) {
    my $y0 = $block[0][1];
    my @new_row;
    while( @block && $block[0][1] - $y0 <= 15 ) {
        push @new_row, shift @block;
    }
    push @row, \@new_row;
}

for my $row (@row) {
    my $line_width = 10000;
    my $i = 0;
    while( $i < $#{$row} ) {
        if( $row->[$i][0] + $row->[$i][2] < $row->[$i+1][0] ) {
            $line_width = min($line_width, $row->[$i+1][0] - ($row->[$i][0]+$row->[$i][2]));
        }
        ++$i;
    }

    #print "y = $row->[0][1]\n";
    #print "line_width = $line_width\n";

    next if( $line_width > 10 );

    $i = 0;
    while( $i < $#{$row} ) {
        if( $row->[$i][0] + $row->[$i][2] + $line_width * 2 + 15 < $row->[$i+1][0] ) {
            splice $row, $i+1, 0, [
                $row->[$i][0] + $row->[$i][2] + $line_width, # x
                $row->[$i][1], #y
                $row->[$i+1][0] - ($row->[$i][0] + $row->[$i][2]) - $line_width * 2, # w
                0,
            ];
            #print "fix @{$row->[$i+1]}\n";
            ++$i;
        }
        ++$i;
    }
}

my @heading_row;
for my $row (@row) {
    @$row = sort { $a->[0] <=> $b->[0] } @$row;
    if( $field_num <= @$row && @$row <= $field_num+2 ) {
        shift $row while( @$row > $field_num && $row->[1][2] < $row->[0][2] * 2.5 );
        pop $row while( @$row > $field_num );
        push @heading_row, $row;
    } elsif( 2*$field_num <= @$row && @$row <= 2*($field_num+2) ) {
        shift $row while( @$row > $field_num && $row->[1][2] < $row->[0][2] * 2.5 );
        push @heading_row, [splice $row, 0, $field_num];

        shift $row while( @$row > $field_num && $row->[1][2] < $row->[0][2] * 2.5 );
        pop $row while( @$row > $field_num );
        push @heading_row, $row;
    }
}

my @final_row;
for my $row (@heading_row) {
    my $height = reduce { max $a, $b->[3] } 0, @$row;
    if( $height > 192 ) {
        for my $b (@$row) {
            $b->[3] = $height;
        }
        push @final_row, $row;
    }
}
@final_row = sort {
    abs( $a->[0][0] - $b->[0][0] ) > 50 && $a->[0][0] <=> $b->[0][0] || $a->[0][1] <=> $b->[0][1]
} @final_row;

if( @final_row ) {
    for my $row (@final_row) {
        for my $i (0..$#field) {
            if( $i==0 ) {
                if( \$row == \$final_row[0] ) {
                    print "[ { ";
                } else {
                    print ", { ";
                }
            } else {
                print "  , ";
            }
            for my $j (0..3) {
                $row->[$i][$j] ||= 0;
            }
            print qq("$field[$i]":\n    [$row->[$i][0], $row->[$i][1], $row->[$i][2], $row->[$i][3]]\n);
        }
        print "  }\n";
    }
    print "]\n";
} else {
    print "[]\n";
}

#print 'heading_row=', 0+@heading_row, $/;
#print 'final_row=', 0+@final_row, $/;
