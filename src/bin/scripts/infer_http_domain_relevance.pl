#!/usr/local/bin/perl -w

if ($#ARGV != 0) {
	print STDERR "usage: $0 YYYY-mm-dd\n";
	exit 1;
}

##### program you want to run
$program_name = "/usr/local/bin/infer_host_domain";

#####################################################################
##### only need to modify below where it says the script starts #####
#####################################################################

##### create pipes to handle communication
pipe (FROM_PERL, TO_PROGRAM);
pipe (FROM_PROGRAM, TO_PERL);

##### create a child process
$pid = fork;
die "Couldn't fork: $!" unless defined $pid;

##### child process becomes the program
if ($pid == 0)  {

	##### attach standard input/output/error to the pipes
	close  STDIN;
	open  (STDIN,  '<&FROM_PERL') || die ("open: $!");

	close  STDOUT;
	open  (STDOUT, '>&TO_PERL')   || die ("open: $!");

	close  STDERR;
	open  (STDERR, '>&STDOUT')	|| die;

	##### close unused parts of pipes
	close FROM_PROGRAM;
	close TO_PROGRAM;

	##### unbuffer the outputs
	select STDERR; $| = 1;
	select STDOUT; $| = 1;

	##### execute the program
	exec $program_name;

	##### shouldn't get here!!!
	die;
}

##### parent process is the perl script
#open (TERMINAL, '>&STDOUT');
#open (KEYBOARD, '<&STDIN');
#
#close STDIN;
#open (STDIN,	'<&FROM_PROGRAM') || die ("open: $!");
#
#close STDOUT;
#open (STDOUT,   '>&TO_PROGRAM')   || die ("open: $!");

close FROM_PERL;
close TO_PERL;

##### unbuffer all the outputs
select FROM_PROGRAM;   $| = 1;
select TO_PROGRAM;   $| = 1;
select STDOUT;   $| = 1;

#####################################################################
#######################  Script Starts Here  ########################
#####################################################################

use Socket;
use DBI;

my $date = $ARGV[0];

my $local_networks_string = `/usr/local/bin/infer_config local-networks 2> /dev/null`;
chomp $local_networks_string;
my @local_networks;
foreach $net_str (split(/\s/, $local_networks_string)) {
	die "invalid local-networks!\n" unless $net_str =~ m|(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})/(\d{1,2})|;
	push(@local_networks, [$1, $2]);
}

foreach my $cidr (@local_networks) {
	${$cidr}[0] = unpack N => pack CCCC => split /\./ => ${$cidr}[0];
	${$cidr}[1] = (2 ** 32) - (2 ** (32 - ${$cidr}[1]));

	die "invalid CIDR!\n" unless (${$cidr}[0] & ${$cidr}[1]) == ${$cidr}[0];
}

sub is_internal {
	my @local_networks = shift;
	my $ip = shift;
	foreach my $cidr (@local_networks) {
		return 1 if ($ip & ${$cidr}[1]) == ${$cidr}[0];
	}
	return 0;
}

my $db_dbname = `/usr/local/bin/infer_config postgresql.dbname 2> /dev/null`;
my $db_port = `/usr/local/bin/infer_config postgresql.port 2> /dev/null`;
my $db_user = `/usr/local/bin/infer_config postgresql.user 2> /dev/null`;
my $db_password = `/usr/local/bin/infer_config postgresql.password 2> /dev/null`;

chomp $db_dbname;
chomp $db_port;
chomp $db_user;
chomp $db_password;

my @db_opts;
if ($db_dbname) {
	push(@db_opts, "dbname=$db_dbname");
}
if ($db_port) {
	push(@db_opts, "port=$db_port");
}

my $dbh = DBI->connect("dbi:Pg:" . join(';', @db_opts),
					   $db_user,
					   $db_password)
    or die "Can't connect to PostgreSQL database: $DBI::errstr\n";

##### This is just an example of what you can do....

#my $query = 'SELECT source_ip, host, destination_ip FROM "HTTPRequests"."' . $date . '" LIMIT 100';
my $query = 'SELECT source_ip, host, destination_ip FROM "HTTPRequests"."' . $date . '"';
my $sth = $dbh->prepare($query)
	or die "Couldn't prepare statement: " . $dbh->errstr;
$sth->execute()
	or die "Couldn't execute statement: " . $sth->errstr;

print STDERR "Executed query: '$query'\n";

my %domains;
my $max_count = 0;
my $cur_count = 0;
while (my ($source_ip, $host_name, $destination_ip) = $sth->fetchrow_array()) {
	next if is_internal(@local_networks, $destination_ip);
	
	if ($host_name ne '') {
		$host_name =~ s/:\d+//;
		print TO_PROGRAM "$host_name\n";
		$host_domain = <FROM_PROGRAM>;
		chop $host_domain;
		if ($host_domain eq '') {
			$host_domain = $host_name;
		}
	}
	else {
		$host_domain = inet_ntoa(pack("N*", $destination_ip));
	}
	$domains{$host_domain}{$source_ip} = 1;
	$cur_count = keys %{$domains{$host_domain}};
	$max_count = $cur_count if $cur_count > $max_count;
}

kill $pid;

$query = 'DROP TABLE IF EXISTS "HTTPDomainRelevance"."' . $date . '"';
$sth = $dbh->prepare($query)
	or die "Couldn't prepare statement: " . $dbh->errstr;
$sth->execute()
	or die "Couldn't execute statement: " . $sth->errstr;

$query = 'CREATE TABLE "HTTPDomainRelevance"."' . $date . '" ' .
			'(domain TEXT, relevance DOUBLE PRECISION)';
$sth = $dbh->prepare($query)
	or die "Couldn't prepare statement: " . $dbh->errstr;
$sth->execute()
	or die "Couldn't execute statement: " . $sth->errstr;


$dbh->do('COPY "HTTPDomainRelevance"."' . $date . '" FROM STDIN')
	or die "Couldn't start COPY: " . $dbh->errstr;

while ( my ($domain, $ips) = each %domains ) {
	$count = keys %$ips;
	$relevance = 1.0 - ($count - 1.0) / $max_count;
#	print "$domain\t$relevance\n";
	$dbh->pg_putcopydata("$domain\t$relevance\n");

#	while (my ($ip, $value) = each %$ips) {
#		print "\t" . inet_ntoa(pack("N*", $ip)) . "\n";
#	}
}
$dbh->pg_putcopyend()
	or die "Couldn't end COPY: " . $dbh->errstr;


$query = 'ALTER TABLE "HTTPDomainRelevance"."' . $date . '" ' .
			'ADD PRIMARY KEY (domain)';
$sth = $dbh->prepare($query)
	or die "Couldn't prepare statement: " . $dbh->errstr;
$sth->execute()
	or die "Couldn't execute statement: " . $sth->errstr;

exit;
