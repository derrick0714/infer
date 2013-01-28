#!/usr/local/bin/perl -w

use DBI;

if ($#ARGV != 0) {
	print STDERR "usage: $0 YYYY-mm-dd\n";
	exit 1;
}

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

my $query = 'SELECT max(internal_host_count) from "ExternalContacts"."' . $date . '"';
my $sth = $dbh->prepare($query)
	or die "Couldn't prepare statement: " . $dbh->errstr;
$sth->execute()
	or die "Couldn't execute statement: " . $sth->errstr;

my $max;
while (my @data = $sth->fetchrow_array()) {
	$max = $data[0];
}
die "No max?" unless $sth->rows == 1;

$update =
	'UPDATE "PortIPs"."' . $date . '" AS A ' .
	'SET relevance = 1 - (cast(cast(B.internal_host_count AS text) AS double precision) - 1) / ? ' .
	'FROM "ExternalContacts"."' . $date . '" AS B ' .
	'WHERE A."externalIP" = B.external_ip';

my $uph = $dbh->prepare($update)
	or die "Couldn't prepare update statement: " . $dbh->errstr;
$uph->execute($max)
	or die "Couldn't execute update statement: " . $uph->errstr;

exit;
