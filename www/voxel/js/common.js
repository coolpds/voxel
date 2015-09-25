var NS4;
var IE4;
if(document.all) {IE4=true;NS4=false;}
else {IE4=false;NS4=true;}

function PageOpen(p)
{
	//var win = window.open(u, "_blank", "");
	var win = window.open(p, "_blank", "height=500,width=600,status=no,toolbar=no,menubar=no,location=no,resizable=yes,scrollbars=no");
	if(win) win.focus();
}

function Trim(str)
{
	return str.replace(/(^\s*)|(\s*$)/g,"").replace(/(\\*$)/g,""); 
}

function SetCookie(name,value,expdays,path,domain)
{
	var ckstr = name + "=" + escape(value) + "; path=" + path + ";";
	if(expdays==0)
	{
		if(domain) ckstr = ckstr + " domain=" + domain + ";";
	}
	else
	{
		var todate = new Date();
		todate.setDate(todate.getDate() + expdays);
		if(domain) ckstr += " expires=" + todate.toGMTString() + "; domain=" + domain + ";";
		else ckstr += " expires=" + todate.toGMTString() + ";";
	}

	document.cookie = ckstr;
}

function GetCookie(name)
{
	var nameOfCookie = name+"=";
	var x=0;
	while (x <= document.cookie.length) 
	{
		var y = (x+nameOfCookie.length);
		if (document.cookie.substring(x, y) == nameOfCookie) 
		{
				if((endOfCookie=document.cookie.indexOf(";", y)) == -1) endOfCookie = document.cookie.length;
				return unescape(document.cookie.substring(y, endOfCookie));
		}
		x = document.cookie.indexOf(" ", x) + 1;
		if (x == 0)
		break;
	}
	return "";
}

function GetHttpRequest()
{
	if(typeof XMLHttpRequest == "undefined") XMLHttpRequest = function()
	{ 
    	try { return new ActiveXObject("Msxml2.XMLHTTP.6.0") } catch(e) {} 
    	try { return new ActiveXObject("Msxml2.XMLHTTP.3.0") } catch(e) {} 
    	try { return new ActiveXObject("Msxml2.XMLHTTP") } catch(e) {} 
    	try { return new ActiveXObject("Microsoft.XMLHTTP") } catch(e) {} 
    	throw new Error("This browser does not support XMLHttpRequest.")
  	};
  	return new XMLHttpRequest();
}

function OnRequestComplete(ajaxh, callbackfunc)
{ 
	if(!ajaxh) return;
	
	if(ajaxh.readyState == 4)
	{
		if(ajaxh.status != 200)
			return;
		
		var json = eval('(' + ajaxh.responseText +')');
		
		if(callbackfunc)
			return callbackfunc(json);
	}
}

function HttpGet(ajaxh, surl, callbackfunc)
{
	if(!ajaxh) return;
	
	try
	{
    ajaxh.open("GET", surl, true);
		ajaxh.onreadystatechange = function() { OnRequestComplete(ajaxh, callbackfunc); };
		ajaxh.send("");
	}
	catch(e)
	{
	}
}

function HttpPost(ajaxh, surl, postdata, callbackfunc)
{
	if(!ajaxh) return;
	
	try
	{
		ajaxh.open("POST", surl, true);
		ajaxh.onreadystatechange = function() { OnRequestComplete(ajaxh, callbackfunc); };
		ajaxh.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
		ajaxh.send(postdata);
	}
	catch(e)
	{
	}
}

function SetPostString(f)
{
	if(!f) return "";
	
	var retstr = "";
	var cnt = f.elements.length;
	for(var i = 0; i< cnt; i++)
	{
		if((f.elements[i].tagName == "SELECT" || f.elements[i].tagName == "select") && f.elements[i].multiple)
		{
			for(var j = 0; j < f.elements[i].options.length; j++)
			{
				if(f.elements[i].options[j].selected)
				{
					if(i < cnt - 1)
					{
						if(f.elements[i].name)
							retstr += f.elements[i].name + "=" + encodeURIComponent(f.elements[i].options[j].value) + "&";
						else if(f.elements[i].id)
							retstr += f.elements[i].id + "=" + encodeURIComponent(f.elements[i].options[j].value) + "&";
					}
					else
					{
						if(f.elements[i].name)
							retstr += f.elements[i].name + "=" + encodeURIComponent(f.elements[i].options[j].value);
						else if(f.elements[i].id)
							retstr += f.elements[i].id + "=" + encodeURIComponent(f.elements[i].options[j].value);
					}
				}
			}
		}
		else
		{
			if(i < cnt - 1)
			{
				if(f.elements[i].value)
				{
					if(f.elements[i].name)
						retstr += f.elements[i].name + "=" + encodeURIComponent(f.elements[i].value) + "&";
					else if(f.elements[i].id)
						retstr += f.elements[i].id + "=" + encodeURIComponent(f.elements[i].value) + "&";
				}
			}
			else
			{
				if(f.elements[i].value)
				{
					if(f.elements[i].name)
						retstr += f.elements[i].name + "=" + encodeURIComponent(f.elements[i].value);
					else if(f.elements[i].id)
						retstr += f.elements[i].id + "=" + encodeURIComponent(f.elements[i].value);
				}
			}
		}
	}
	
	return retstr;
}

function ReplaceAll(str, srchs, repls)
{
    while(str.indexOf(srchs) != -1)
		str = str.replace(srchs, repls);

    return str;
}

function ArrMax(arr)
{
	if(!arr) return	null;
	return Math.max.apply(Math, arr);
}

function ArrMin(arr)
{
	if(!arr) return	null;
	return Math.min.apply(Math, arr);	
}

function NumberFormat(number, decimals, dec_point, thousands_sep)
{
	number = (number + '').replace(/[^0-9+\-Ee.]/g, '');
	var n = !isFinite(+number) ? 0 : +number,
		prec = !isFinite(+decimals) ? 0 : Math.abs(decimals), sep = (typeof thousands_sep === 'undefined') ? ',' : thousands_sep,
		dec = (typeof dec_point === 'undefined') ? '.' : dec_point,
		s = '',
		toFixedFix = function (n, prec)
		{
			var k = Math.pow(10, prec);
			return '' + Math.round(n * k) / k;
		};
	s = (prec ? toFixedFix(n, prec) : '' + Math.round(n)).split('.');
	if(s[0].length > 3)
	{
		s[0] = s[0].replace(/\B(?=(?:\d{3})+(?!\d))/g, sep);
    }
	if((s[1] || '').length < prec)
	{
		s[1] = s[1] || '';
		s[1] += new Array(prec - s[1].length + 1).join('0');
	}
	return s.join(dec);
}
