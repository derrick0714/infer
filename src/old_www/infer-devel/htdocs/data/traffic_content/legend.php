<?php
$legend = array(
	"traffic_content_types" => array(
		"Plaintext", 
		"BMP image", 
		"WAV audio", 
		"Compressed", 
		"JPEG image", 
		"MP3 audio", 
		"MPEG video", 
		"Encrypted", 
		"Unknown"
	)
);

echo json_encode($legend);
?>