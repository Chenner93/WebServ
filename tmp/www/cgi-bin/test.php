<?php
// test_post.php - version minimaliste
error_reporting(E_ALL);

$input = file_get_contents('php://input');
header('Content-Type: text/plain');
header('Status: 200');
echo('');
echo "POST body reçu: " . strlen($input) . " octets\n";
echo "Contenu: $input\n";
?>