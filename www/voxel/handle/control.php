<?php
function execCmd($cmd)
{
	$execstr = "";
	$retval = 0;
	$strarr = "";
	$tmpstr = "";

	@exec("nohup ".$cmd." 2>&1", $strarr, $retval);
	foreach($strarr as $tmpstr) $execstr .= $tmpstr."\n";
	return array($retval, $execstr);
}

$cmd = @strtolower(@$_GET["cmd"]);
$txt = @$_GET["txt"];
$pid = @intval(@file_get_contents("/tmp/vox.pid"));
$sig = 0;
$r = array();

if($pid <= 1 || $pid > 65535)
{
	$r["errcode"] = "-1";
	$r["errmsg"] = "no available server";
	$r["command"] = $cmd;
	echo json_encode($r);
	exit;
}

$cmds = array(
		"fl"=>34,"ff"=>35,"fr"=>36,
		"ll"=>37,"st1"=>38,"st2"=>39,"rr"=>40,
		"bl"=>41,"bb"=>42,"br"=>43,
		"cc"=>44,"ut"=>45,"cw"=>46,
		"up"=>47,"ac5"=>48,"ac4"=>49,"ac3"=>50,"ac2"=>51,"ac1"=>52,"dn"=>53,
		"clt"=>54,"crt"=>55,"cup"=>56,"cdn"=>57,
		"ee"=>58,"fb"=>59,/*tts*/
		"m1"=>64,"m2"=>63,"m3"=>62,"m4"=>61,"m5"=>60
		);

if(array_key_exists($cmd, $cmds))
{
	$sig = $cmds[$cmd];
}
else
{
	$signo = @intval($cmd);
	
	if($sino >= 34 && $signo <= 64)
		$sig = $signo;
}

if(!$sig || $sig == 0)
{
	$r["errcode"] = "-2";
	$r["errmsg"] = "unrecognized command";
	$r["command"] = $cmd;
	echo json_encode($r);
	exit;
}

// tts
if($sig === 59)
  @file_put_contents("/tmp/tts.txt", $txt);

list($ret, $retmsg) = execCmd("sudo /bin/kill -".$sig." ".$pid);
//$ret = @trim(@shell_exec("nohup sudo /bin/kill -".$sig." ".$pid." > /dev/null; echo $?"));

if($ret !== 0)
{
	$r["errcode"] = "-3";
	$r["errmsg"] = "signal sending failed, ".$pid.":".$sig.":".$retmsg;
	$r["command"] = $cmd;
	echo json_encode($r);
	exit;
}

$r["errcode"] = "0";
$r["errmsg"] = "";
$r["command"] = $cmd;
echo json_encode($r);
exit;

?>