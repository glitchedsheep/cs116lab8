<?php
	session_start();
	if ($_SESSION['login'] == null)
	{
		header("Location: admin.php");
		exit;
	}
	if (!isset($_COOKIE['admin'])) {
	   setcookie('admin', 'false');
	   $_COOKIE['admin'] = 'false';
	   echo '
<html>
<head><title>404 Not Found</title></head>
<body bgcolor="white">
<center><h1>404 Not Found</h1><p>Have you visited <a href="https://34.30.233.19/">Eval Homer</a>?</.p></center>
<hr><center>nginx/1.14.12</center>
</body>
</html>
<!-- Hmmm, the plot thickens... key{a2197e5fed07c1fb32e9bf8ef2d9c3542ad71d45cad3be9099bd5b4b184e4184}-->';
     }
     elseif (isset($_COOKIE['admin']) && strcmp($_COOKIE['admin'], 'true') == 0) {
     	    echo "<!DOCTYPE html><html><head><title>Main</title></head><body><p>Congratulations! Here you go: key{d416765b9913d93801cb50fdec8fe7c7aaa3578fd723147f703644f26fcb1947}</p></body></html>";
     }
     else {
                echo '
<html>
<head><title>404 Not Found</title></head>
<body bgcolor="white">
<center><h1>404 Not Found</h1><p>Have you visited <a href="https://34.30.233.19/">Eval Homer</a>?</.p></center>
<hr><center>nginx/1.14.12</center>
</body>
</html>
<!-- Hmmm, the plot thickens... key{a2197e5fed07c1fb32e9bf8ef2d9c3542ad71d45cad3be9099bd5b4b184e4184}-->';}
?>
