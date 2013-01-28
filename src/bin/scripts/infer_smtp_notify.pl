#!/usr/local/bin/perl -w

if ($#ARGV != 0) {
	print STDERR "usage: $0 YYYY-mm-dd\n";
	exit 1;
}

my $date = $ARGV[0];

use DBI;
use Socket;
use Net::SMTP_auth;
use POSIX qw(strftime);

my $management_host =
	`/usr/local/bin/infer_config management-host 2> /dev/null`;
chomp $management_host;
my $smtp_server =
	`/usr/local/bin/infer_config smtp_notify.smtp-server 2> /dev/null`;
chomp $smtp_server;
my $smtp_user =
	`/usr/local/bin/infer_config smtp_notify.smtp-user 2> /dev/null`;
chomp $smtp_user;
my $smtp_password =
	`/usr/local/bin/infer_config smtp_notify.smtp-password 2> /dev/null`;
chomp $smtp_password;

my $smtp_displayname =
	`/usr/local/bin/infer_config smtp_notify.smtp-displayname 2> /dev/null`;
chomp $smtp_displayname;
my $smtp_from =
	`/usr/local/bin/infer_config smtp_notify.smtp-from 2> /dev/null`;
chomp $smtp_from;

my $subject_prefix =
	`/usr/local/bin/infer_config smtp_notify.subject-prefix 2> /dev/null`;
chomp $subject_prefix;

my $to = `/usr/local/bin/infer_config smtp_notify.to 2> /dev/null`;
chomp $to;

my $ip = '128.238.9.103';
my $body = 'This is a test email! Yay if it works...';

# We need to query postgres to get the necessary info.
my $db_dbname =
	`/usr/local/bin/infer_config postgresql.dbname 2> /dev/null`;
chomp $db_dbname;
my $db_port =
	`/usr/local/bin/infer_config postgresql.port 2> /dev/null`;
chomp $db_port;
my $db_user =
	`/usr/local/bin/infer_config postgresql.user 2> /dev/null`;
chomp $db_user;
my $db_password =
	`/usr/local/bin/infer_config postgresql.password 2> /dev/null`;
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

# determine the top 20 hosts
my $limit = 20;
my $query =
	'SELECT rank, ip ' .
	'FROM "InterestingIPs"."' . $date . '" '.
	'ORDER BY rank ASC ' .
	'LIMIT ' . $limit;
my $sth = $dbh->prepare($query)
	or die "Couldn't prepare statement: " . $dbh->errstr;
$sth->execute()
	or die "Couldn't execute statement: " . $sth->errstr;

#print STDERR "Executed query: '$query'\n";

my @interesting_ips;
while (my ($rank, $interesting_ip) = $sth->fetchrow_array()) {
	push @interesting_ips, [$rank, $interesting_ip];
}

# TODO for each of the top hosts, formulate an email akin to /host/#/events
# Connect to the SMTP server
my $smtp = Net::SMTP_auth->new($smtp_server,
							   Hello => 'infer',
							   Debug => 1,);

@auth_types = $smtp->auth_types();

my $auth_success = 0;
my $auth_method;
foreach $_ (@auth_types) {
	$auth_method = $_;
	$auth_success = $smtp->auth($auth_method, $smtp_user, $smtp_password)
		or print STDERR "Authentication method '$auth_method' failed!\n";
	last if $auth_success;
}

die "All Authentication attempts have failed!\n" unless $auth_success;

print STDERR "Authentication successful using method '$auth_method'...\n";

foreach my $cur (@interesting_ips) {
	$rank = ${$cur}[0];
	$interesting_ip = ${$cur}[1];
	$interesting_ip_text = inet_ntoa(pack("N*", ${$cur}[1]));
	$subject = sprintf "$date: (%02d/$limit) $interesting_ip_text", $rank;

	$query =
			"SELECT pi.\"internalIP\" as internal_ip, " .
				   "ms.name, " .
				   "CAST(CAST(COUNT(pi.\"externalIP\") AS text) AS uint64) AS num_contacted, " .
				   "CAST(CAST(SUM(pi.\"numBytes\") AS text) AS uint64) AS num_bytes, " .
				   "MIN(pi.\"startTime\") AS start_time, " .
				   "MAX(pi.\"endTime\") AS end_time " .
			"FROM \"PortIPs\".\"" . $date . "\" AS pi " .
			"INNER JOIN \"Maps\".\"monitoredServices\" AS ms " .
			"ON pi.\"internalIP\" = '" . $interesting_ip . "' " .
				"AND ms.ports @> ARRAY[pi.port] " .
				"AND ms.initiator = pi.initiator " .
			"GROUP BY pi.\"internalIP\", " .
					 "ms.name " .
		"UNION " .
			"SELECT ip as internal_ip, " .
				   "rn.role_name as name, " .
				   "CAST(CAST(\"numHosts\" AS text) AS uint64) as num_contacted, " .
				   "\"numBytes\" AS num_bytes, " .
				   "\"startTime\" AS start_time, " .
				   "\"endTime\" AS end_time " .
			"FROM \"Roles\".\"" . $date . "\" AS r " .
			"INNER JOIN \"Maps\".role_names AS rn " .
			"ON r.role = rn.role_id " .
			"WHERE ip = '" . $interesting_ip . "' " .
		"UNION " .
			"SELECT \"internalIP\" AS internal_ip, " .
				   "'Infected Contacts' AS name, " .
				   "CAST(CAST(COUNT(DISTINCT \"externalIP\") AS text) AS uint64) AS num_contacted, " .
				   "CAST(CAST(SUM(\"numBytes\") AS text) AS uint64) AS num_bytes, " .
				   "MIN(\"startTime\") AS start_time, " .
				   "MAX(\"endTime\") AS end_time " .
			"FROM \"InfectedContacts\".\"" . $date . "\" " .
			"WHERE \"internalIP\" = '" . $interesting_ip . "' " .
			"GROUP BY \"internalIP\" " .
		"UNION " .
			"SELECT \"internalIP\" AS internal_ip, " .
				   "'Evasive Traffic' AS name, " .
				   "CAST(CAST(COUNT(DISTINCT \"externalIP\") AS text) AS uint64) AS num_contacted, " .
				   "CAST(CAST(SUM(\"numBytes\") AS text) AS uint64) AS num_bytes, " .
				   "MIN(\"startTime\") AS start_time, " .
				   "MAX(\"endTime\") AS end_time " .
			"FROM \"EvasiveTraffic\".\"" . $date . "\" " .
			"WHERE \"internalIP\" = '" . $interesting_ip . "' " .
			"GROUP BY \"internalIP\" " .
		"UNION " .
			"SELECT \"sourceIP\" AS internal_ip, " .
				   "'Dark Space Source' AS name, " .
				   "CAST(CAST(COUNT(DISTINCT \"destinationIP\") AS text) AS uint64) AS num_contacted, " .
				   "CAST(CAST(SUM(\"numBytes\") AS text) AS uint64) AS num_bytes, " .
				   "MIN(\"startTime\") AS start_time, " .
				   "MAX(\"endTime\") AS end_time " .
			"FROM \"DarkSpaceSources\".\"" . $date . "\" " .
			"WHERE \"sourceIP\" = '" . $interesting_ip . "' " .
			"GROUP BY \"sourceIP\" " .
		"UNION " .
			"SELECT \"internalIP\" AS internal_ip, " .
				   "'Dark Space Target' AS name, " .
				   "CAST(CAST(COUNT(DISTINCT \"externalIP\") AS text) AS uint64) AS num_contacted, " .
				   "CAST(CAST(SUM(\"numBytes\") AS text) AS uint64) AS num_bytes, " .
				   "MIN(\"startTime\") AS start_time, " .
				   "MAX(\"endTime\") AS end_time " .
			"FROM \"DarkSpaceTargets\".\"" . $date . "\" " .
			"WHERE \"internalIP\" = '" . $interesting_ip . "' " .
			"GROUP BY \"internalIP\" " .
		"UNION " .
			"SELECT \"internalIP\" AS internal_ip, " .
				   "'Protocol Violations' AS name, " .
				   "CAST(CAST(COUNT(DISTINCT \"externalIP\") AS text) AS uint64) AS num_contacted, " .
				   "CAST(CAST(SUM(\"numBytes\") AS text) AS uint64) AS num_bytes, " .
				   "MIN(\"startTime\") AS start_time, " .
				   "MAX(\"endTime\") AS end_time " .
			"FROM \"NonDNSTraffic\".\"" . $date . "\" " .
			"WHERE \"internalIP\" = '" . $interesting_ip . "' " .
			"GROUP BY \"internalIP\" ";

	$sth = $dbh->prepare($query)
		or die "Couldn't prepare statement: " . $dbh->errstr;
	$sth->execute()
		or die "Couldn't execute statement: " . $sth->errstr;
	#print STDERR "Executed query: '$query'\n";

	my $width = 18;
	
	$body =
		sprintf "Infer event report for %s on %s\n",
			$interesting_ip_text, $date;
	$body .=
		sprintf "https://%s/host/#/events/%s/%s\n",
			$management_host, $interesting_ip_text, $date;
	$body .=
		"\n\n";

	while (my ($internal_ip,
			   $indicator_name,
			   $num_contacted,
			   $num_bytes,
			   $start_time,
			   $end_time) = $sth->fetchrow_array())
	{
		$indicator = lc $indicator_name;
		$indicator =~ s/ /_/g;
		$body .=
			sprintf "https://%s/host/#/indicators/%s/%s/%s\n",
				$management_host, $interesting_ip_text, $date, $indicator;
		$body .=
			sprintf "  %-${width}s %s\n", "Indicator:", $indicator_name;
		$body .=
			sprintf "  %-${width}s %s\n", "Hosts contacted:", $num_contacted;
		$body .=
			sprintf "  %-${width}s %s\n", "Bytes transferred:", $num_bytes;
		$body .=
			sprintf "  %-${width}s %s\n", "First occurrence:",
				strftime("%a, %d %b %Y %H:%M:%S %z", localtime($start_time));
		$body .=
			sprintf "  %-${width}s %s\n", "Last occurrence:",
				strftime("%a, %d %b %Y %H:%M:%S %z", localtime($end_time));
		$body .=
			"\n";
	}

	$smtp->mail($smtp_from);
	$smtp->to($to);

	$smtp->data;
	$smtp->datasend("From: $smtp_displayname <$smtp_from>\n");
	$smtp->datasend("To: $to\n");
	$smtp->datasend("Subject: $subject_prefix $subject\n");
	$smtp->datasend("Date: " . strftime("%a, %d %b %Y %H:%M:%S %z", localtime()) . "\n");
	$smtp->datasend("Message-ID: <infer_alert\_$date\_$ip\@$smtp_server>\n");
	$smtp->datasend("\n");
	$smtp->datasend($body);
	$smtp->dataend;

	sleep 1;
}

$smtp->quit;

exit;
