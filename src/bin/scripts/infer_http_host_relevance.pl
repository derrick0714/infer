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

# This one for the domain
my $domh = DBI->connect("dbi:Pg:" . join(';', @db_opts),
					   $db_user,
					   $db_password)
    or die "Can't connect to PostgreSQL database: $DBI::errstr\n";

# This one for the COPY
my $cph = DBI->connect("dbi:Pg:" . join(';', @db_opts),
					   $db_user,
					   $db_password)
    or die "Can't connect to PostgreSQL database: $DBI::errstr\n";

my $query = 'DROP TABLE IF EXISTS "HTTPHostRelevance"."' . $date . '"';
my $sth = $dbh->prepare($query)
	or die "Couldn't prepare statement: " . $dbh->errstr;
$sth->execute()
	or die "Couldn't execute statement: " . $sth->errstr;

$query = 'CREATE TABLE "HTTPHostRelevance"."' . $date . '" ' .
			'(host TEXT, relevance DOUBLE PRECISION)';
$sth = $dbh->prepare($query)
	or die "Couldn't prepare statement: " . $dbh->errstr;
$sth->execute()
	or die "Couldn't execute statement: " . $sth->errstr;


$query = 'SELECT distinct host ' .
		 'FROM "HTTPRequests"."' . $date . '"';
$sth = $dbh->prepare($query)
	or die "Couldn't prepare statement: " . $dbh->errstr;
$sth->execute()
	or die "Couldn't execute statement: " . $sth->errstr;

$query = 'SELECT relevance ' .
		 'FROM "HTTPDomainRelevance"."' . $date . '" ' .
		 'WHERE domain = ?';
my $domsth = $domh->prepare($query)
	or die "Couldn't prepare statement: " . $domh->errstr;

$cph->do('COPY "HTTPHostRelevance"."' . $date . '" FROM STDIN')
	or die "Couldn't start COPY: " . $dbh->errstr;
while (my ($orig_host_name) = $sth->fetchrow_array()) {
	next if $orig_host_name eq '';
	#if ($host_name ne '') {
	my $host_name = $orig_host_name;
	$host_name =~ s/:\d+//;
	print TO_PROGRAM "$host_name\n";
	$host_domain = <FROM_PROGRAM>;
	chop $host_domain;
	if ($host_domain eq '') {
		$host_domain = $host_name;
	}
	#}
	#else {
	#	$host_domain = inet_ntoa(pack("N*", $destination_ip));
	#}
	$domsth->execute($host_domain)
		or die "Couldn't execute statement: " . $domsth->errstr;
	my ($relevance) = $domsth->fetchrow_array();
	if (defined $relevance) {
		$cph->pg_putcopydata("$orig_host_name\t$relevance\n");
	}
}
$cph->pg_putcopyend()
	or die "Couldn't end COPY: " . $cph->errstr;

$query = 'ALTER TABLE "HTTPHostRelevance"."' . $date . '" ' .
			'ADD PRIMARY KEY (host)';
$sth = $dbh->prepare($query)
	or die "Couldn't prepare statement: " . $dbh->errstr;
$sth->execute()
	or die "Couldn't execute statement: " . $sth->errstr;

$query = 'CREATE INDEX "' . $date . '_relevance" ON ' .
			'"HTTPHostRelevance"."' . $date . '" (relevance)';
$sth = $dbh->prepare($query)
	or die "Couldn't prepare statement: " . $dbh->errstr;
$sth->execute()
	or die "Couldn't execute statement: " . $sth->errstr;

$query = 'DROP INDEX IF EXISTS ' .
			'"HTTPRequests"."' . $date . '_source_ip_host"';
$sth = $dbh->prepare($query)
	or die "Couldn't prepare statement: " . $dbh->errstr;
$sth->execute()
	or die "Couldn't execute statement: " . $sth->errstr;

$query = 'CREATE INDEX "' . $date . '_source_ip_host" ON ' .
			'"HTTPRequests"."' . $date . '" (source_ip, host)';
$sth = $dbh->prepare($query)
	or die "Couldn't prepare statement: " . $dbh->errstr;
$sth->execute()
	or die "Couldn't execute statement: " . $sth->errstr;

exit;
