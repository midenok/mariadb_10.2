package My::Suite::Versioning;
use My::Platform;

@ISA = qw(My::Suite);

sub skip_combinations {
  my %skip;
  unless ($::mysqld_variables{'innodb'} eq "ON" or $ENV{HA_INNODB_SO})
  {
    $skip{'engines.combinations'} = [ 'innodb', 'myisam' ];
  }

  %skip;
}

bless { };

