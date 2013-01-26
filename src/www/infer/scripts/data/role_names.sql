--
-- PostgreSQL database dump
--

SET client_encoding = 'SQL_ASCII';
SET standard_conforming_strings = off;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET escape_string_warning = off;

SET search_path = "Maps", pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: role_names; Type: TABLE; Schema: Maps; Owner: ims; Tablespace: 
--

CREATE TABLE role_names (
    role_name text,
    role_short text,
    role_id public.uint16
);


ALTER TABLE "Maps".role_names OWNER TO ims;

--
-- Data for Name: role_names; Type: TABLE DATA; Schema: Maps; Owner: ims
--

INSERT INTO role_names VALUES ('Dark Space Bot', 'dark_space_bot', '0');
INSERT INTO role_names VALUES ('Spam Bot', 'spam_bot', '1');
INSERT INTO role_names VALUES ('Mail Server Bot', 'mail_server_bot', '2');
INSERT INTO role_names VALUES ('Web Server Bot', 'web_server_bot', '3');
INSERT INTO role_names VALUES ('Multimedia P2P Node', 'multimedia_p2p_node', '4');
INSERT INTO role_names VALUES ('Encrypted P2P Node', 'encrypted_p2p_node', '13');
INSERT INTO role_names VALUES ('FTP Brute Forcer', 'ftp_brute_forcer', '5');
INSERT INTO role_names VALUES ('FTP Brute Forced', 'ftp_brute_forced', '6');
INSERT INTO role_names VALUES ('SSH Brute Forcer', 'ssh_brute_forcer', '7');
INSERT INTO role_names VALUES ('SSH Brute Forced', 'ssh_brute_forced', '8');
INSERT INTO role_names VALUES ('Telnet Brute Forcer', 'telnet_brute_forcer', '9');
INSERT INTO role_names VALUES ('Telnet Brute Forced', 'telnet_brute_forced', '10');
INSERT INTO role_names VALUES ('Scan Bot', 'scan_bot', '11');
INSERT INTO role_names VALUES ('Unclassified P2P Node', 'unclassified_p2p_node', '12');
INSERT INTO role_names VALUES ('Microsoft SQL Brute Forcer', 'microsoft_sql_brute_forcer', '14');
INSERT INTO role_names VALUES ('Microsoft SQL Brute Forced', 'microsoft_sql_brute_forced', '15');
INSERT INTO role_names VALUES ('Oracle SQL Brute Forcer', 'oracle_sql_brute_forcer', '16');
INSERT INTO role_names VALUES ('Oracle SQL Brute Forced', 'oracle_sql_brute_forced', '17');
INSERT INTO role_names VALUES ('mySQL Brute Forcer', 'mysql_brute_forcer', '18');
INSERT INTO role_names VALUES ('mySQL Brute Forced', 'mysql_brute_forced', '19');
INSERT INTO role_names VALUES ('PostgreSQL Brute Forcer', 'postgresql_brute_forcer', '20');
INSERT INTO role_names VALUES ('PostgreSQL Brute Forced', 'postgresql_brute_forced', '21');


--
-- PostgreSQL database dump complete
--

