<?php

$dat = @file_get_contents("/tmp/vox.dat");

if(!$dat)
{
	$r["errcode"] = "-1";
	$r["errmsg"] = "no data file";
	echo json_encode($r);
  exit;
}

$d = @explode("|", $dat);

if(count($d) < 10)
{
  $r["errcode"] = "-2";
	$r["errmsg"] = "invalid data";
	echo json_encode($r);
  exit;
}

$r["errcode"] = "0";
$r["errmsg"] = "";
$r["myip"] = trim($d[0]);
$r["mode"] = trim($d[1]);
$r["stat"] = trim($d[2]);
$r["freeze"] = trim($d[3]);
$r["temperature"] = trim($d[4]);
$r["humidity"] = trim($d[5]);
$r["dist_f"] = trim($d[6]);
$r["dist_b"] = trim($d[7]);
$r["dist_l"] = trim($d[8]);
$r["dist_r"] = trim($d[9]);
$r["cpuusage"] = trim($d[10]);
$r["cputemp"] = trim($d[11]);
$r["appmem"] = trim($d[12]);
$r["follow"] = trim($d[13]);
$r["ball_x"] = trim($d[14]);
$r["ball_y"] = trim($d[15]);
$r["ball_r"] = trim($d[16]);
$r["ball_cnt"] = trim($d[17]);
$r["record"] = trim($d[18]);
$r["reqmsg"] = trim($d[19]);
$r["rspmsg"] = trim($d[20]);

echo json_encode($r);
exit;

?>