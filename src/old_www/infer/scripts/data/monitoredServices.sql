--
-- PostgreSQL database dump
--

SET client_encoding = 'SQL_ASCII';
SET standard_conforming_strings = off;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET escape_string_warning = off;

SET search_path = "Maps", pg_catalog;

--
-- Data for Name: monitoredServices; Type: TABLE DATA; Schema: Maps; Owner: ims
--

COPY "monitoredServices" (name, ports, initiator) FROM stdin;
Mail Server	{25,110,143}	2
Secure Mail Server	{465,993,995}	2
Web Server	{80}	2
Secure Web Server	{443}	2
Mail Client	{25,110,143}	1
Secure Mail Client	{465,993,995}	1
Web Client	{80}	1
Secure Web Client	{443}	1
\.


--
-- PostgreSQL database dump complete
--

