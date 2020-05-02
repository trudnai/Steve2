#!/usr/bin/perl

my $addr = "";
my $status = 0;
my @buf = ();

my $max = 5000;



while(<>) {
    if ($status == 0) {
        if (/([0-9A-F]{4}):( [0-9A-F]{2}){3}\s*LDA \$C08C,X/) {
#            print("DEBUG: 0 LDA\n");
            $status = 1;
            push(@buf, $_);
            $addr = $1;
        }
        else {
#            print("DEBUG: 0 no LDA\n");
            print;
        }
    }
    elsif ($status == 1) {
        if (/BPL \$$addr/) {
#            print("DEBUG: 1 BPL\n");
            $status = 2;
            push(@buf, $_);
        }
        else {
#            print("DEBUG: 1 no BPL\n");
            $status = 0;
            print @buf;
            @buf = ();
            print;
        }
    }
    elsif ($status == 2) {
        if (/EOR #\$D5/) {
#            print("DEBUG: 2 EOR\n");
            $status = 3;
            push(@buf, $_);
        }
        else {
#            print("DEBUG: 2 no EOR\n");
            $status = 0;
            print @buf;
            @buf = ();
            print;
        }
    }
    elsif ($status == 3) {
        if (/BNE \$$addr/) {
#            print("DEBUG: 3 BNE\n");
            $status = 4;
            push(@buf, $_);
        }
        else {
#            print("DEBUG: 3 no BNE\n");
            $status = 0;
            print @buf;
            @buf = ();
            print;
        }
    }
    elsif ($status == 4) {
        if (/$addr:( [0-9A-F]{2}){3}\s*LDA \$C08C,X/) {
#            print("DEBUG: 4 LDA\n");
            $status = 1;
            @buf = ();
            push(@buf, $_);
        }
        else {
#            print("DEBUG: 4 no LDA\n");
            $status = 0;
            print @buf, "\n";
            @buf = ();
            print;
        }
    }
    else {
#        print("DEBUG: else\n");
        $status = 0;
        print @buf;
        @buf = ();
        print;
    }

#    if( -- $max <= 0 ) {
#        last;
#    }
}

