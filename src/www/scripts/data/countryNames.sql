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
-- Data for Name: countryNames; Type: TABLE DATA; Schema: Maps; Owner: ims
--

COPY "countryNames" ("countryNumber", "countryCode", "countryName") FROM stdin;
4	AF	Afghanistan
248	AX	�land Islands
8	AL	Albania
12	DZ	Algeria
16	AS	American Samoa
20	AD	Andorra
24	AO	Angola
660	AI	Anguilla
10	AQ	Antarctica
28	AG	Antigua and Barbuda
32	AR	Argentina
51	AM	Armenia
533	AW	Aruba
36	AU	Australia
40	AT	Austria
31	AZ	Azerbaijan
44	BS	Bahamas
48	BH	Bahrain
50	BD	Bangladesh
52	BB	Barbados
112	BY	Belarus
56	BE	Belgium
84	BZ	Belize
204	BJ	Benin
60	BM	Bermuda
64	BT	Bhutan
68	BO	Bolivia
70	BA	Bosnia and Herzegovina
72	BW	Botswana
74	BV	Bouvet Island
76	BR	Brazil
86	IO	British Indian Ocean Territory
96	BN	Brunei Darussalam
100	BG	Bulgaria
854	BF	Burkina Faso
108	BI	Burundi
116	KH	Cambodia
120	CM	Cameroon
124	CA	Canada
132	CV	Cape Verde
136	KY	Cayman Islands
140	CF	Central African Republic
148	TD	Chad
152	CL	Chile
156	CN	China
162	CX	Christmas Island
166	CC	Cocos (Keeling) Islands
170	CO	Colombia
174	KM	Comoros
178	CG	Congo
180	CD	Congo, the Democratic Republic of the
184	CK	Cook Islands
188	CR	Costa Rica
384	CI	Cote D'ivoire
191	HR	Croatia
192	CU	Cuba
196	CY	Cyprus
203	CZ	Czech Republic
208	DK	Denmark
262	DJ	Djibouti
212	DM	Dominica
214	DO	Dominican Republic
218	EC	Ecuador
818	EG	Egypt
222	SV	El Salvador
226	GQ	Equatorial Guinea
232	ER	Eritrea
233	EE	Estonia
231	ET	Ethiopia
238	FK	Falkland Islands (Malvinas)
234	FO	Faroe Islands
242	FJ	Fiji
246	FI	Finland
250	FR	France
254	GF	French Guiana
258	PF	French Polynesia
260	TF	French Southern Territories
266	GA	Gabon
270	GM	Gambia
268	GE	Georgia
276	DE	Germany
288	GH	Ghana
292	GI	Gibraltar
300	GR	Greece
304	GL	Greenland
308	GD	Grenada
312	GP	Guadeloupe
316	GU	Guam
320	GT	Guatemala
324	GN	Guinea
624	GW	Guinea-Bissau
328	GY	Guyana
332	HT	Haiti
334	HM	Heard Island and Mcdonald Islands
336	VA	Holy See (Vatican City State)
340	HN	Honduras
344	HK	Hong Kong
348	HU	Hungary
352	IS	Iceland
356	IN	India
360	ID	Indonesia
364	IR	Iran, Islamic Republic of
368	IQ	Iraq
372	IE	Ireland
376	IL	Israel
380	IT	Italy
388	JM	Jamaica
392	JP	Japan
400	JO	Jordan
398	KZ	Kazakhstan
404	KE	Kenya
296	KI	Kiribati
408	KP	Korea, Democratic People's Republic of
410	KR	Korea, Republic of
414	KW	Kuwait
417	KG	Kyrgyzstan
418	LA	Lao People's Democratic Republic
428	LV	Latvia
422	LB	Lebanon
426	LS	Lesotho
430	LR	Liberia
434	LY	Libyan Arab Jamahiriya
438	LI	Liechtenstein
440	LT	Lithuania
442	LU	Luxembourg
446	MO	Macao
807	MK	Macedonia, the Former Yugoslav Republic of
450	MG	Madagascar
454	MW	Malawi
458	MY	Malaysia
462	MV	Maldives
466	ML	Mali
470	MT	Malta
584	MH	Marshall Islands
474	MQ	Martinique
478	MR	Mauritania
480	MU	Mauritius
175	YT	Mayotte
484	MX	Mexico
583	FM	Micronesia, Federated States of
498	MD	Moldova, Republic of
492	MC	Monaco
496	MN	Mongolia
500	MS	Montserrat
504	MA	Morocco
508	MZ	Mozambique
104	MM	Myanmar
516	NA	Namibia
520	NR	Nauru
524	NP	Nepal
528	NL	Netherlands
530	AN	Netherlands Antilles
540	NC	New Caledonia
554	NZ	New Zealand
558	NI	Nicaragua
562	NE	Niger
566	NG	Nigeria
570	NU	Niue
574	NF	Norfolk Island
580	MP	Northern Mariana Islands
578	NO	Norway
512	OM	Oman
586	PK	Pakistan
585	PW	Palau
275	PS	Palestinian Territory, Occupied
591	PA	Panama
598	PG	Papua New Guinea
600	PY	Paraguay
604	PE	Peru
608	PH	Philippines
612	PN	Pitcairn
616	PL	Poland
620	PT	Portugal
630	PR	Puerto Rico
634	QA	Qatar
638	RE	Reunion
642	RO	Romania
643	RU	Russian Federation
646	RW	Rwanda
654	SH	Saint Helena
659	KN	Saint Kitts and Nevis
662	LC	Saint Lucia
666	PM	Saint Pierre and Miquelon
670	VC	Saint Vincent and the Grenadines
882	WS	Samoa
674	SM	San Marino
678	ST	Sao Tome and Principe
682	SA	Saudi Arabia
686	SN	Senegal
891	CS	Serbia and Montenegro
690	SC	Seychelles
694	SL	Sierra Leone
702	SG	Singapore
703	SK	Slovakia
705	SI	Slovenia
90	SB	Solomon Islands
706	SO	Somalia
710	ZA	South Africa
239	GS	South Georgia and the South Sandwich Islands
724	ES	Spain
144	LK	Sri Lanka
736	SD	Sudan
740	SR	Suriname
744	SJ	Svalbard and Jan Mayen
748	SZ	Swaziland
752	SE	Sweden
756	CH	Switzerland
760	SY	Syrian Arab Republic
158	TW	Taiwan, Province of China
762	TJ	Tajikistan
834	TZ	Tanzania, United Republic of
764	TH	Thailand
626	TL	Timor-Leste
768	TG	Togo
772	TK	Tokelau
776	TO	Tonga
780	TT	Trinidad and Tobago
788	TN	Tunisia
792	TR	Turkey
795	TM	Turkmenistan
796	TC	Turks and Caicos Islands
798	TV	Tuvalu
800	UG	Uganda
804	UA	Ukraine
784	AE	United Arab Emirates
826	GB	United Kingdom
840	US	United States
581	UM	United States Minor Outlying Islands
858	UY	Uruguay
860	UZ	Uzbekistan
548	VU	Vanuatu
862	VE	Venezuela
704	VN	Viet Nam
92	VG	Virgin Islands, British
850	VI	Virgin Islands, U.S.
876	WF	Wallis and Futuna
732	EH	Western Sahara
887	YE	Yemen
894	ZM	Zambia
716	ZW	Zimbabwe
-7	A1	Anonymous Proxy
-2	A2	Satellite Provider
-3	AP	Asia/Pacific Region
-4	EU	Europe
-5	FX	France, Metropolitan
-6	ZR	Zaire
0	\N	Unknown
-1	ZZ	IETF Reserved Address Space
688	RS	Serbia
\.


--
-- PostgreSQL database dump complete
--
