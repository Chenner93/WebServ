<?php
header("Content-Type: text/html; charset=UTF-8");
header('Status: 200');
?>
<!DOCTYPE html>
<html>
<head>
    <title>Hello PHP</title>
</head>
<body>
    <h1>Hello from PHP CGI!</h1>
    <p><?php echo "Current time: " . date('Y-m-d H:i:s'); ?></p>
</body>
</html>
