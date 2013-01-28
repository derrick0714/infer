<?php
session_set_cookie_params('3600', '/', '', true);
session_name("INFER");
session_regenerate_id(true); 

session_start(); 
session_destroy();

session_start(); 
$_SESSION['redir'] = $_SERVER['HTTP_REFERER'];
header('Location: /login/');
?>
