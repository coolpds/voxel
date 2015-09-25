<?
$db_host = 'localhost';
//$db_port = '';
//$db_sock = ":$prefix/var/mysql.sock";
$db_user = 'root';
$db_pw = '';
$db_name = 'vox';

$connect=@mysql_connect($db_host, $db_user, $db_pw);
@mysql_select_db($db_name, $connect);

?>