<?php

$voxcmd = "ps aux | grep vox | grep -v voxel | grep -v .sh | grep -v grep | awk '{print $2}' | head -n 1";
//echo $cmdstr."<br />";
@exec("nohup ".$voxcmd, $voxout);
$voxpid = @intval($voxout[0]);

if($voxpid <= 0 || $voxpid >= 65535)
{
  $voxstart = "sudo /usr/local/bin/vox.sh";
  @exec("nohup ".$voxstart." 2>&1", $voxout);
  sleep(2);
}

$cmd = @strtolower(@$_GET["cmd"]);
$start = "/usr/local/bin/start_mjpg.sh";
$stop = "/usr/local/bin/stop_mjpg.sh";
$cmdstr = "ps aux | grep mjpg_streamer | grep -v grep | awk '{print $2}' | head -n 1";
//echo $cmdstr."<br />";

@exec("nohup ".$cmdstr, $out);
$pid = @intval($out[0]);
//echo "pid".$pid;

if($cmd == "stop")
{
  //if($pid > 0 && $pid < 65535)
  //  @exec("nohup ".$stop." 2>&1", $out);
}
else if($cmd == "start")
{
  if($pid < 1)
  {
    $voxpid = @intval(@file_get_contents("/tmp/vox.pid"));

    if($voxpid > 1 && $voxpid < 65535)
    {
      @exec("nohup sudo ls -l /proc/".$voxpid."/fd ", $out);

      foreach($out as $v)
      {
        if(strpos($v, "/dev/video") != FALSE)
        {
          $r["errcode"] = "-2";
          $r["errmsg"] = "already used by voxel";
          $r["command"] = $cmd;
          echo json_encode($r);
          exit;
        }
      }
    }
    
    @exec("nohup ".$start." 2>&1", $out);
    sleep(2);
  }
}
else
{
  $r["errcode"] = "-1";
  $r["errmsg"] = "invalid command";
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