<?php
$gateway = @$_GET["gateway"];

if($gateway != "")
{
  echo $gateway."({\"messages\": \"ok\"})";
  exit;
}
@header('cache-control: no-cache');
?>
<html>
<head>
<meta http-equiv="Cache-Control" content="no-cache" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta http-equiv="Content-Script-Type" content="text/javascript" />
<meta http-equiv="Content-Style-Type" content="text/css" />
<meta http-equiv="X-UA-Compatible" content="IE=edge" />
<!--meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, minimum-scale=0, user-scalable=no, target-densitydpi=medium-dpi" /-->
<meta name="mobile-web-app-capable" content="yes">
<meta name=”apple-mobile-web-app-capable” content=”yes”>
<title>Voxel Controller</title>
<link rel="stylesheet" type="text/css" href="css/style.css" />
<script type="text/javascript" src="js/common.js"></script>
<script type="text/javascript">
var ajax = GetHttpRequest();
var map = {'up':0,'dn':0,'fl':0,'ff':0,'fr':0,'ll':0,'st1':0,'st2':0,'rr':0,'bl':0,'bb':0,'br':0,'cc':0,'ut':0,'cw':0,'clt':0,'crt':0,'cup':0,'cdn':0};
var acc = 3;
var v_on = false;

function speak()
{
  var obj = document.getElementById("txt");
  if(!obj) return;
  
  var txt = obj.value;
  if(!txt || txt == "") return;
  
  HttpGet(ajax, "/voxel/handle/control.php?cmd=fb&txt="+txt, cbControl);
  obj.value = "";
}

function control(cmd)
{
  HttpGet(ajax, "/voxel/handle/control.php?cmd="+cmd, cbControl);
}

function cbControl(j)
{
	if(!j) return;
	
	if(j.errcode != "0")
    alert(j.errcode + ": " + j.errmsg);
}

function webcam()
{
  if(v_on)
    HttpGet(ajax, "/voxel/handle/video.php?cmd=stop", cbWebcam);
  else
    HttpGet(ajax, "/voxel/handle/video.php?cmd=start", cbWebcam);
}

function cbWebcam(j)
{
  if(!j) return;
	
	if(j.errcode != "0")
	{
    alert(j.errcode + ": " + j.errmsg);
    return;
  }

  v_on = !v_on;
  
  var ovid = document.getElementById("vid_panel");
  var port = 9000;

  if(window.location.port == 81)
    port = 9001;

  if(v_on)
    ovid.src = "http://"+window.location.hostname+":"+port+"/?action=stream";
  else
    ovid.src = "img/dog.png";
    
  //ovid.style['-webkit-transform'] = 'rotate(180deg)';
  //ovid.style['transform'] = 'rotate(180deg)';
  //ovid.style['-ms-transform'] = 'rotate(180deg)';
}

function onMEvent(n, onoff)
{
  var n_on = document.getElementById(n + "_on");
  var n_off = document.getElementById(n + "_off");

  if(!n_on || !n_off)
    return;

  var val = map[n];

  if(val == null || val == "undefined")
    return;
    
  if(val != onoff)
  {
    map[n] = onoff;

    if(onoff)
    {
      n_off.style.display = "none";
      n_on.style.display = "inline";
      
      if(n == "up" || n == "dn")
      {
        if(event.type == "mousedown")
        {
          if(n == "up" && acc < 5)
            ++acc;
          else if(n == "dn" && acc > 1)
            --acc;
          
          for(var i = 1; i < 6; i++)
          {
            var ac_on = document.getElementById("ac" + i.toString() + "_on");
            var ac_off = document.getElementById("ac" + i.toString() + "_off");
            
            if(!ac_on || !ac_off)
              continue;
            
            if(acc == i)
            {
              ac_off.style.display = "none";
              ac_on.style.display = "inline";
              control("ac"+i.toString());
            }
            else
            {
              ac_on.style.display = "none";
              ac_off.style.display = "inline";
            }
          }
        }
      }
      else
        control(n);
    }
    else
    {
      n_on.style.display = "none";
      n_off.style.display = "inline";
      
      if(n != "up" && n != "dn" && n != "st2")
        control("st1");
    }
  }
}

function orient()
{
  switch(window.orientation)
  {
    case 0:
      document.body.style.setProperty("-webkit-transform", "rotate(90deg)", null);
      document.body.style.setProperty("-ms-transform", "rotate(90deg)", null);
      document.body.style.setProperty("transform", "rotate(90deg)", null);
      document.body.style.marginTop = "10px";
      document.body.style.marginLeft = "-250px";
      document.body.style.marginRight = "20px";
      break;
    case 90:
      document.body.style.setProperty("-webkit-transform", "rotate(0deg)", null);
      document.body.style.setProperty("-ms-transform", "rotate(0deg)", null);
      document.body.style.setProperty("transform", "rotate(0deg)", null);
      document.body.style.marginTop = "0px";
      document.body.style.marginLeft = "0px";
      document.body.style.marginRight = "0px";
      break;
    case -90:
      document.body.style.setProperty("-webkit-transform", "rotate(0deg)", null);
      document.body.style.setProperty("-ms-transform", "rotate(0deg)", null);
      document.body.style.setProperty("transform", "rotate(0deg)", null);
      document.body.style.marginTop = "0px";
      document.body.style.marginLeft = "0px";
      document.body.style.marginRight = "0px";
      break;
  }
}

//window.onload = orient;
</script>
</head>
<body bgcolor="black" oncontextmenu="return false;" ondrag="return false;" ontouchmove="return false;" ondragstart="return false;" ondragend="return false;" onselectstart="return false;">

<table border=0 width=330 height=250 cellpadding=0 cellspacing=0 align=center>
<tr><td width=330 height=5></td></tr>
<tr><td width=330 height=240 align=center>
<img id="vid_panel" name="vid_panel" src="img/dog.png" width=320 height=240 border=0 onmousedown="webcam()" />
<!--img id="vid_panel" name="vid_panel" src="img/dog2.png" width=320 height=240 border=0 onmousedown="video_flask()" style="-ms-transform: rotate(180deg); -webkit-transform: rotate(180deg); transform: rotate(180deg);"/-->
</td></tr>
<tr><td width=330 height=1></td></tr>
</table>

<table border=0 width=330 height=220 cellpadding=0 cellspacing=0 align=center>
<tr>
<td width=90 align=center>

<table border=0 width=70 cellpadding=0 cellspacing=0>
<tr>
<td width=47 height=30 onmousedown="onMEvent('up',1)" ontouchstart="onMEvent('up',1)" onmouseup="onMEvent('up',0)" onmouseleave="onMEvent('up',0)" ontouchend="onMEvent('up',0)">
<img id="up_off" src="img/up_off.png" width=47 height=30 style="display:inline" />
<img id="up_on" src="img/up_on.png" width=47 height=30 style="display:none" />
</td>
</tr>
<tr><td height=5></td></tr>
<tr>
<td width=47 height=30>
<img id="ac5_off" src="img/ac5_off.png" width=47 height=20 style="display:inline" />
<img id="ac5_on" src="img/ac5_on.png" width=47 height=20 style="display:none" />
</td>
</tr>
<tr>
<td width=47 height=30>
<img id="ac4_off" src="img/ac4_off.png" width=47 height=20 style="display:inline" />
<img id="ac4_on" src="img/ac4_on.png" width=47 height=20 style="display:none" />
</td>
</tr>
<tr>
<td width=47 height=30>
<img id="ac3_off" src="img/ac3_off.png" width=47 height=20 style="display:none" />
<img id="ac3_on" src="img/ac3_on.png" width=47 height=20 style="display:inline" />
</td>
</tr>
<tr>
<td width=47 height=30>
<img id="ac2_off" src="img/ac2_off.png" width=47 height=20 style="display:inline" />
<img id="ac2_on" src="img/ac2_on.png" width=47 height=20 style="display:none" />
</td>
</tr>
<tr>
<td width=47 height=30>
<img id="ac1_off" src="img/ac1_off.png" width=47 height=20 style="display:inline" />
<img id="ac1_on" src="img/ac1_on.png" width=47 height=20 style="display:none" />
</td>
</tr>
<tr><td height=5></td></tr>
<tr>
<td width=47 height=30 onmousedown="onMEvent('dn',1)" ontouchstart="onMEvent('dn',1)" onmouseup="onMEvent('dn',0)" onmouseleave="onMEvent('dn',0)" ontouchend="onMEvent('dn',0)">
<img id="dn_off" src="img/dn_off.png" width=47 height=30 style="display:inline" />
<img id="dn_on" src="img/dn_on.png" width=47 height=30 style="display:none" />
</td>
</tr>
</table>

</td>
<td width=20></td>
<td align=center>

<table border=0 width=195 height=52 cellpadding=0 cellspacing=0>
<tr align=center>
<td width=65 onmousedown="onMEvent('fl',1)" ontouchstart="onMEvent('fl',1)" onmouseup="onMEvent('fl',0)" onmouseleave="onMEvent('fl',0)" ontouchend="onMEvent('fl',0)">
<img id="fl_off" src="img/fl_off.png" width=41 height=42 style="margin:10 0 0 10; display:inline" />
<img id="fl_on" src="img/fl_on.png" width=41 height=42 style="margin:10 0 0 10; display:none" />
</td>
<td width=65 onmousedown="onMEvent('ff',1)" ontouchstart="onMEvent('ff',1)" onmouseup="onMEvent('ff',0)" onmouseleave="onMEvent('ff',0)" ontouchend="onMEvent('ff',0)">
<img id="ff_off" src="img/ff_off.png" width=44 height=46 style="margin:0 0 0 0; display:inline" />
<img id="ff_on" src="img/ff_on.png" width=44 height=46 style="margin:0 0 0 0; display:none" />
</td>
<td width=65 onmousedown="onMEvent('fr',1)" ontouchstart="onMEvent('fr',1)" onmouseup="onMEvent('fr',0)" onmouseleave="onMEvent('fr',0)" ontouchend="onMEvent('fr',0)">
<img id="fr_off" src="img/fr_off.png" width=41 height=43 style="margin:0 0 -10 -10; display:inline" />
<img id="fr_on" src="img/fr_on.png" width=41 height=43 style="margin:0 0 -10 -10; display:none" />
</td>
</tr>
</table>

<table border=0 width=195 height=52 cellpadding=0 cellspacing=0>
<tr align=center>
<td width=65 onmousedown="onMEvent('ll',1)" ontouchstart="onMEvent('ll',1)" onmouseup="onMEvent('ll',0)" onmouseleave="onMEvent('ll',0)" ontouchend="onMEvent('ll',0)">
<img id="ll_off" src="img/ll_off.png" width=46 height=41 style="margin:0 0 0 0; display:inline" />
<img id="ll_on" src="img/ll_on.png" width=46 height=41 style="margin:0 0 0 0; display:none" />
</td>
<td width=65 onmousedown="onMEvent('st2',1)" ontouchstart="onMEvent('st2',1)" onmouseup="onMEvent('st2',0)" onmouseleave="onMEvent('st2',0)" ontouchend="onMEvent('st2',0)">
<img id="st2_off" src="img/st2_off.png" width=40 height=40 style="margin:0 0 0 2; display:inline" />
<img id="st2_on" src="img/st2_on.png" width=40 height=40 style="margin:0 0 0 2; display:none" />
</td>
<td width=65 onmousedown="onMEvent('rr',1)" ontouchstart="onMEvent('rr',1)" onmouseup="onMEvent('rr',0)" onmouseleave="onMEvent('rr',0)" ontouchend="onMEvent('rr',0)">
<img id="rr_off" src="img/rr_off.png" width=48 height=45 style="margin:0 0 0 0; display:inline" />
<img id="rr_on" src="img/rr_on.png" width=48 height=45 style="margin:0 0 0 0; display:none" />
</td>
</tr>
</table>

<table border=0 width=195 height=52 cellpadding=0 cellspacing=0>
<tr align=center>
<td width=65 onmousedown="onMEvent('bl',1)" ontouchstart="onMEvent('bl',1)" onmouseup="onMEvent('bl',0)" onmouseleave="onMEvent('bl',0)" ontouchend="onMEvent('bl',0)">
<img id="bl_off" src="img/bl_off.png" width=42 height=42 style="margin:0 0 10 10; display:inline" />
<img id="bl_on" src="img/bl_on.png" width=42 height=42 style="margin:0 0 10 10; display:none" />
</td>
<td width=65 onmousedown="onMEvent('bb',1)" ontouchstart="onMEvent('bb',1)" onmouseup="onMEvent('bb',0)" onmouseleave="onMEvent('bb',0)" ontouchend="onMEvent('bb',0)">
<img id="bb_off" src="img/bb_off.png" width=44 height=48 style="margin:0 0 0 0; display:inline" />
<img id="bb_on" src="img/bb_on.png" width=44 height=48 style="margin:0 0 0 0; display:none" />
</td>
<td width=65 onmousedown="onMEvent('br',1)" ontouchstart="onMEvent('br',1)" onmouseup="onMEvent('br',0)" onmouseleave="onMEvent('br',0)" ontouchend="onMEvent('br',0)">
<img id="br_off" src="img/br_off.png" width=43 height=42 style="margin:0 0 2 -10; display:inline" />
<img id="br_on" src="img/br_on.png" width=43 height=42 style="margin:0 0 2 -10; display:none" />
</td>
</tr>
</table>

<table border=0 width=195 height=52 cellpadding=0 cellspacing=0>
<tr><td height=5></td></tr>
<tr align=center>
<td width=65 onmousedown="onMEvent('cc',1)" ontouchstart="onMEvent('cc',1)" onmouseup="onMEvent('cc',0)" onmouseleave="onMEvent('cc',0)" ontouchend="onMEvent('cc',0)">
<img id="cc_off" src="img/cc_off.png" width=50 height=47 style="margin:0 0 0 0; display:inline" />
<img id="cc_on" src="img/cc_on.png" width=50 height=47 style="margin:0 0 0 0; display:none" />
</td>
<td width=65 onmousedown="onMEvent('ut',1)" ontouchstart="onMEvent('ut',1)" onmouseup="onMEvent('ut',0)" onmouseleave="onMEvent('ut',0)" ontouchend="onMEvent('ut',0)">
<img id="ut_off" src="img/ut_off.png" width=37 height=55 style="margin:0 0 0 3; display:inline" />
<img id="ut_on" src="img/ut_on.png" width=37 height=55 style="margin:0 0 0 3; display:none" />
</td>
<td width=65 onmousedown="onMEvent('cw',1)" ontouchstart="onMEvent('cw',1)" onmouseup="onMEvent('cw',0)" onmouseleave="onMEvent('cw',0)" ontouchend="onMEvent('cw',0)">
<img id="cw_off" src="img/cw_off.png" width=49 height=48 style="margin:0 0 0 3; display:inline" />
<img id="cw_on" src="img/cw_on.png" width=49 height=48 style="margin:0 0 0 3; display:none" />
</td>
</tr>
</table>

</td>
</tr>
</table>

<table border=0 width=330 height=50 cellpadding=0 cellspacing=0 align=center>
<tr><td height=5></td></tr>
<tr align=center>
<td width=25 onmousedown="webcam()" align=center><b><font color=red>C</font><br /><font color=lightgreen>A</font><br /><font color=deepskyblue>M</font></b></td>
<td width=15></td>
<td width=65 onmousedown="onMEvent('clt',1)" ontouchstart="onMEvent('clt',1)" onmouseup="onMEvent('clt',0)" onmouseleave="onMEvent('clt',0)" ontouchend="onMEvent('clt',0)">
<img id="clt_off" src="img/clt_off.png" width=46 height=41 style="margin:0 0 0 0; display:inline" />
<img id="clt_on" src="img/clt_on.png" width=46 height=41 style="margin:0 0 0 0; display:none" />
</td>
<td width=65 onmousedown="onMEvent('cup',1)" ontouchstart="onMEvent('cup',1)" onmouseup="onMEvent('cup',0)" onmouseleave="onMEvent('cup',0)" ontouchend="onMEvent('cup',0)">
<img id="cup_off" src="img/cup_off.png" width=41 height=42 style="margin:0 0 0 0; display:inline" />
<img id="cup_on" src="img/cup_on.png" width=41 height=42 style="margin:0 0 0 0; display:none" />
</td>
<td width=65 onmousedown="onMEvent('cdn',1)" ontouchstart="onMEvent('cdn',1)" onmouseup="onMEvent('cdn',0)" onmouseleave="onMEvent('cdn',0)" ontouchend="onMEvent('cdn',0)">
<img id="cdn_off" src="img/cdn_off.png" width=44 height=48 style="margin:0 0 0 0; display:inline" />
<img id="cdn_on" src="img/cdn_on.png" width=44 height=48 style="margin:0 0 0 0; display:none" />
</td>
<td width=65 onmousedown="onMEvent('crt',1)" ontouchstart="onMEvent('crt',1)" onmouseup="onMEvent('crt',0)" onmouseleave="onMEvent('crt',0)" ontouchend="onMEvent('crt',0)">
<img id="crt_off" src="img/crt_off.png" width=48 height=45 style="margin:0 0 0 0; display:inline" />
<img id="crt_on" src="img/crt_on.png" width=48 height=45 style="margin:0 0 0 0; display:none" />
</td>
</tr>
</table>

<form id="speaking_form" name="speaking_form" onsubmit="speak(); return false;">
<table border=0 width=330 height=48 cellpadding=0 cellspacing=0 align=center>
<tr>
<td><input name="txt" id="txt" type="text" autocomplete="off" lang="kr" dir="ltr" spellcheck="false" style="width:100%;height:28px;padding-left:5px;font-weight:bold;" value=""></input></td>
<td width="50"><input type=button onclick="speak();" value=" speak " style="width:100%;height:28px;"></input></td>
</tr>
</table>
</form>

</body>
</html>
