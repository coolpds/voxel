<?php
@header('cache-control: no-cache');
?>
<html>
<head>
<meta http-equiv="Cache-Control" content="no-cache" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta http-equiv="Content-Script-Type" content="text/javascript" />
<meta http-equiv="Content-Style-Type" content="text/css" />
<meta http-equiv="X-UA-Compatible" content="IE=edge" />
<meta name="mobile-web-app-capable" content="yes">
<meta name=”apple-mobile-web-app-capable” content=”yes”>
<title>Voxel Controller</title>
<link rel="stylesheet" type="text/css" href="css/style.css" />
<script type="text/javascript" src="js/common.js"></script>
<script type="text/javascript">
var ajax = GetHttpRequest();
var v_on = false;
var map = {34:'FL',35:'FF',36:'FR',37:'LL',38:'ST1',39:'ST2',40:'RR',41:'BL',42:'BB',43:'BR',44:'CC',45:'UT',46:'CW',47:'UP',48:'AC5',49:'AC4',50:'AC3',51:'AC2',52:'AC1',53:'DN',54:'CLT',55:'CRT',56:'CUP',57:'CDN',58:'EE',59:'FB'};
var ee_timer = null;
var ee_cnt = 0;
var v_on = false;

function control(cmd)
{
  var ah = GetHttpRequest();
  HttpGet(ah, "/voxel/handle/control.php?cmd=" + cmd, cbControl);
}

function cbControl(j)
{
	if(!j) return;
	
	if(j.errcode != "0")
	{
    //alert(j.errcode + ": " + j.errmsg);
    var rspmsg = document.getElementById("rspmsg");
    if(rspmsg) rspmsg.innerText = j.errmsg;
    var tbl_msg = document.getElementById("tbl_msg");
    if(tbl_msg) tbl_msg.style.zIndex = "300";
  }
}

function webcam()
{
  var ah = GetHttpRequest();
 
  if(v_on)
    HttpGet(ah, "/voxel/handle/video.php?cmd=stop", cbWebcam);
  else
    HttpGet(ah, "/voxel/handle/video.php?cmd=start", cbWebcam);
}

function cbWebcam(j)
{
  if(!j) return;

  var ovid = document.getElementById("img_vid");
  var port = 9000;
  	
	if(j.errcode != "0")
	{
    //alert(j.errcode + ": " + j.errmsg);
    ovid.style.display = "none";
    var rspmsg = document.getElementById("rspmsg");
    if(rspmsg) rspmsg.innerText = j.errmsg;
    var tbl_msg = document.getElementById("tbl_msg");
    if(tbl_msg) tbl_msg.style.zIndex = "300";
    return;
  }

  v_on = !v_on;
  
  if(window.location.port == 81)
    port = 9001;

  if(v_on)
  {
    ovid.src = "http://"+window.location.hostname+":"+port+"/?action=stream&thumb";
    ovid.style.display = "inline";
  }
  else
  {
    ovid.src = "/voxel/img/null.png";
    ovid.style.display = "none";
    location.reload();
  }
}

function incEasterEggCnt()
{
  if(ee_timer)
    clearTimeout(ee_timer);
  
  ee_cnt = 0;
}

function bodyClick()
{
  if(ee_timer)
    clearTimeout(ee_timer);
  
  if(ee_cnt > 3)
  {
    ee_cnt = 0;
    control("ee");
    //window.location.href = "/starwars/crawling.html";
    setTimeout("window.location.href = '/starwars/crawling.html'", 500);
  }
  else
  {  
    ee_cnt++;
    ee_timer = setTimeout("incEasterEggCnt()", 1000);
  }
}

function status()
{
  HttpGet(ajax, "/voxel/handle/status.php", cbStatus);
}

function cbStatus(j)
{
  setTimeout("status()", 300);

	if(!j)
	{
    document.getElementById("outtbl").style.backgroundColor = "gray";
    document.getElementById("record_on").style.display = "none";
    document.getElementById("record_off").style.display = "inline";
    document.getElementById("fb_start").style.display = "none";
    document.getElementById("fb_stop").style.display = "none";
    return;
  }
	
	if(j.errcode != "0")
	{
    document.getElementById("outtbl").style.backgroundColor = "gray";
    document.getElementById("record_on").style.display = "none";
    document.getElementById("record_off").style.display = "inline";
    document.getElementById("fb_start").style.display = "none";
    document.getElementById("fb_stop").style.display = "none";
    return;
  }
  
  var myip = document.getElementById("myip");
  if(myip) myip.innerHTML = j.myip;
  
  var mode = document.getElementById("mode");
  var mno = parseInt(j.mode);
  if(mno == 1)
    mode.innerHTML = "1 (컨트롤)";
  else if(mno == 2)
    mode.innerHTML = "2 (비전)";
  else if(mno == 3)
    mode.innerHTML = "3 (보이스)";
  else if(mno == 4)
    mode.innerHTML = "4 (TODO)";
  else if(mno == 5)
    mode.innerHTML = "5 (TODO)";
  else
    mode.innerHTML = "Unknown";
    
  var stat = document.getElementById("stat");
  var sno = parseInt(j.stat);
  var val = map[sno];
  if(val != null && val != "undefined")
    stat.innerHTML = val;
  
  var freeze = parseInt(j.freeze);
  if(freeze == 0) document.getElementById("outtbl").style.backgroundColor = "darkgreen";
  else document.getElementById("outtbl").style.backgroundColor = "red";
  
  var dist_f = document.getElementById("dist_f");
  if(dist_f)
  {
    if(parseFloat(j.dist_f) < 40.0)
      dist_f.innerHTML = "<font color=red>"+j.dist_f+"</font>";
    else
      dist_f.innerHTML = j.dist_f;
  }
  
  var dist_b = document.getElementById("dist_b");
  if(dist_b)
  {
    if(parseFloat(j.dist_b) > 4.0)
      dist_b.innerHTML = "<font color=red>"+j.dist_b+"</font>";
    else
      dist_b.innerHTML = j.dist_b;
  }
  
  var dist_l = document.getElementById("dist_l");
  if(dist_l)
  {
    if(parseFloat(j.dist_l) < 20.0)
      dist_l.innerHTML = "<font color=red>"+j.dist_l+"</font>";
    else
      dist_l.innerHTML = j.dist_l;
  }
  
  var dist_r = document.getElementById("dist_r");
  if(dist_r)
  {
    if(parseFloat(j.dist_r) < 20.0)
      dist_r.innerHTML = "<font color=red>"+j.dist_r+"</font>";
    else
      dist_r.innerHTML = j.dist_r;
  }
  
  var temperature = document.getElementById("temperature");
  if(temperature) temperature.innerHTML = j.temperature;
  
  var humidity = document.getElementById("humidity");
  if(humidity) humidity.innerHTML = j.humidity;
  
  var cpuusage = document.getElementById("cpuusage");
  if(cpuusage)
  {
    if(parseFloat(j.cputemp) > 75.0)
      cpuusage.innerHTML = "<font color=red>"+j.cpuusage+"</font>";
    else
      cpuusage.innerHTML = j.cpuusage;
  }
  
  var cputemp = document.getElementById("cputemp");
  if(cputemp)
  {
    if(parseFloat(j.cputemp) > 55.0)
      cputemp.innerHTML = "<font color=red>"+j.cputemp+"</font>";
    else
      cputemp.innerHTML = j.cputemp;
  }
  
  var appmem = document.getElementById("appmem");
  if(appmem) appmem.innerHTML = j.appmem;

  var ball_info = document.getElementById("ball_info");
  var ball_name = document.getElementById("ball_name");
  var fb_start = document.getElementById("fb_start");
  var fb_stop = document.getElementById("fb_stop");

  if(mno != 2)
  {
    if(ball_info) ball_info.innerHTML = "<font color=gray>No vision mode</font>";
    if(ball_name) ball_name.innerHTML = "볼";
    if(fb_start) fb_start.style.display = "none";
    if(fb_stop) fb_stop.style.display = "none"; 
  }
  else
  {
    if(parseFloat(j.ball_r) > 0.0)
    {
      if(ball_info) ball_info.innerHTML = j.ball_x + " x " + j.ball_y + ", " + j.ball_r + " / " + j.ball_cnt;
      if(ball_name) ball_name.innerHTML = "<font color=lightgreen>볼</font>";
    }
    else
    {
      if(ball_info) ball_info.innerHTML = "<font color=darkgray>Not detected!</font>";
      if(ball_name) ball_name.innerHTML = "볼";
    }

    if(fb_start && fb_stop)
    {
      if(v_on)
      {
        fb_start.style.opacity = "0.7";
        fb_stop.style.opacity = "0.7"; 
      }
      else
      {
        fb_start.style.opacity = "1.0";
        fb_stop.style.opacity = "1.0"; 
      }
            
      if(parseInt(j.follow))
      {
        fb_start.style.display = "none";
        fb_stop.style.display = "inline"; 
      }
      else
      {
        fb_stop.style.display = "none"; 
        fb_start.style.display = "inline";
      }
    }
  }

  var ball_x = document.getElementById("ball_x");
  if(ball_x) ball_x.innerHTML = j.ball_x;

  var ball_y = document.getElementById("ball_y");
  if(ball_y) ball_y.innerHTML = j.ball_y;

  var ball_r = document.getElementById("ball_r");
  if(ball_r) ball_r.innerHTML = j.ball_r;
 
  var ball_cnt = document.getElementById("ball_cnt");
  if(ball_cnt) ball_cnt.innerHTML = j.ball_cnt;
  
  var record_off = document.getElementById("record_off");
  var record_on = document.getElementById("record_on");
  
  if(record_off && record_on)
  {
    if(mno == 3 && parseInt(j.record) == 1)
    {
      record_off.style.display = "none";
      record_on.style.display = "inline";
    }
    else
    {
      record_on.style.display = "none";
      record_off.style.display = "inline";
    }
  }

  var reqmsg = document.getElementById("reqmsg");
  var rspmsg = document.getElementById("rspmsg");
  var tbl_msg = document.getElementById("tbl_msg");
  
  if(mno == 3)
  {
    tbl_msg.style.zIndex = "300";
  
    if(reqmsg)
    {
      if(j.reqmsg != "")
        reqmsg.innerText = "<<< " + j.reqmsg;
      else
        reqmsg.innerText = "";
    }
    
    if(rspmsg)
    {
      if(j.rspmsg != "")
        rspmsg.innerText = ">>> " + j.rspmsg;
      else
        rspmsg.innerText = "";
    }
  }
  else
  {
    tbl_msg.style.zIndex = "0";
    if(reqmsg) reqmsg.innerText = "";
    if(rspmsg) rspmsg.innerText = "";
  }
}
</script>
</head>
<body onload="status();" onmouseup="bodyClick()" style="overflow:hidden;" bgcolor="black" oncontextmenu="return false;" ondrag="return false;" ontouchmove="return false;" ondragstart="return false;" ondragend="return false;" onselectstart="return false;">

<table id=outtbl border=0 width=240 height=320 style="table-layout:fixed;" bgcolor=darkgreen>
<tr>
<td align=center valign=middle>

<table border=0 width=226 height=310 bgcolor=black>
<tr>
<td valign=top>

<table border=0 width=226 height=2><tr><td align=center></td></tr></table>
<table border=0 width=226><tr><td align=center>F: <b><span id="dist_f">0.0</span></b> cm</td></tr></table>
<table border=0 width=226 style="margin-top:-11px;padding-left:4px;padding-right:4px"><tr><td width=115>L: <b><span id="dist_l">0.0</span></b> cm</td><td width=115 align=right>R: <b><span id="dist_r">0.0</span></b> cm</td></tr></table>
<table border=0 width=226 style="margin-top:-11px"><tr><td align=center>B: <b><span id="dist_b">0.0</span></b> cm</td></tr></table>

<table border=0 width=226>
<tr><td width=55 align=right><font color=yellow>모드</font>: </td><td>&nbsp;<span id=mode>1 (컨트롤)</span>&nbsp; <font color=green>상태</font>: <span id=stat>ST1</span></td></tr>
<tr><td width=55 align=right>MyIP: </td><td>&nbsp;<span id=myip>127.0.0.1</span></td></tr>
<tr><td width=55 align=right>CPU: </td><td>&nbsp;<span id=cpuusage>0.0</span> % / <span id=cputemp>40</span> ℃</td></tr>
<tr><td width=55 align=right>메모리: </td><td>&nbsp;<span id=appmem>0</span> mb&nbsp; 전압: <span id=volt>3.3</span> v</td></tr>
<tr><td width=55 align=right>온도: </td><td>&nbsp;<span id=temperature>0</span> ℃&nbsp; 습도: <span id=humidity>0</span> %</td></tr>
<tr><td width=55 align=right><span id=ball_name>볼</span>: </td><td>&nbsp;<span id=ball_info>Not detected!</span></td></tr>
</table>

<img id="record_off" src="img/mic_off.gif" width=42 height=42 style="position:absolute;top:185px;left:13px;z-index:101;display:inline" />
<img id="record_on" src="img/mic_ani.gif" width=42 height=42 style="position:absolute;top:185px;left:13px;z-index:102;display:none" />
<span id=cam_btn style="position:absolute;top:242px;left:22px;z-index:100;display:inline"><a class="btn_gray" onclick="webcam()"><font color=red>C</font><br /><font color=green>A</font><br /><font color=blue>M</font></a></span>
<img id="img_vid" src="/voxel/img/null.png" width=160 height=120 border=0 style="position:absolute;top:186px;left:66px;z-index:99;display:none;" onclick="webcam()" />

<span id=fb_start style="position:absolute;top:210px;left:115px;z-index:201;display:none;background:#000;filter:alpha(opacity=7);opacity:0.7;-moz-opacity:0.7;"><a class="btn_green" onclick="control('fb')">&nbsp;Follow&nbsp;</a></span>
<span id=fb_stop style="position:absolute;top:210px;left:115px;z-index:202;display:none;background:#000;filter:alpha(opacity=7);opacity:0.7;-moz-opacity:0.7;"><a class="btn_red" onclick="control('fb')">&nbsp;&nbsp;Stop&nbsp;&nbsp;</a></span>

<table id=tbl_msg border=0 width=165 height=120 style="position:absolute;top:187px;left:64px;z-index:0;table-layout:fixed;border:0 solid white;">
<tr><td><span id=reqmsg style="font-size:13px;font-weight:bold;color:#3DB7CC;line-height:1.4;"></span></td></tr>
<tr><td><span id=rspmsg style="font-size:13px;font-weight:bold;color:#CCA63D;line-height:1.4;"></span></td></tr>
</table>

</td>
</tr>
</table>

</td>
</tr>
</table>

</body>
</html>