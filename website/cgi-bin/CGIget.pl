use strict;
use warnings;

my $value = 1;  # Perl uses 1 for true and 0 for false

if ($value) {
    print "Hello everybody\n";
}

# while (1) {
#     print "Coucou\n";
# }

my $path = $ENV{'PATH_INFO'};
print "$path\n";
