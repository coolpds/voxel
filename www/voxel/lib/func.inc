<?
error_reporting(E_ALL);
ini_set('display_errors','On');
date_default_timezone_set('Asia/Seoul');

session_start();

function GoBack($msg="") { 
  echo "<script language=javascript>\n<!--\n"; 
  if($msg)echo "alert('".$msg."');\n"; 
  echo "history.back();\n"; 
  echo "// -->\n</script>\n";
}

$ldpath = "";

function AppendLDLibPath($path)
{
	global $ldpath;
	$ldpath = getenv("LD_LIBRARY_PATH");
	//TraceLog($ldpath);
	$newld = $path;
	if($ldpath) $newld .= ":$ldpath";
	putenv("LD_LIBRARY_PATH=$newld");
	
	//TraceLog(getenv("LD_LIBRARY_PATH"));
}

function RestoreLDLibPath()
{
	global $ldpath;
	//TraceLog($ldpath);
	putenv("LD_LIBRARY_PATH=$ldpath");
	$ldpath = "";
}

function GetLDLibPath()
{
	return getenv("LD_LIBRARY_PATH");
}

function PsExists($pid)
{
	if(!$pid)
		return false;
	
	$command = "ps -p ".$pid;
	exec($command, $strarr);
	
	if(isset($strarr[1]))
		return true;
        
	return false;
}

function UTF8CutStr($str,$len,$tail='..')
{
	$c = @substr(@str_pad(@decbin(@ord($str{$len})),8,'0',STR_PAD_LEFT),0,2);
	if($c == '10') for (;$c != '11' && $c{0} == 1;$c = @substr(@str_pad(@decbin(@ord($str{--$len})),8,'0',STR_PAD_LEFT),0,2));
	return @substr($str,0,$len).(@strlen($str)-@strlen($tail) >= $len ? $tail : '');
}

function ShortenDateTime($date)
{
	$shorten = "";
	$datetime = @mktime(@substr($date, 11, 2),@substr($date, 14, 2),@substr($date, 17, 2),@substr($date, 5, 2),@substr($date, 8, 2),@substr($date, 0, 4));
	$current = time();
	if(intval($current) - intval($datetime) >= 86400) $shorten = @substr($date, 0, 10);
	else $shorten = @substr($date, 11, 8);
	return $shorten;
}

function Xml2Array($url, $get_attributes = 1, $priority = 'tag')
{
	if(!@function_exists('xml_parser_create')) return array();
	
	$stream = @fopen($url, "r");
	if(!$stream) return array();
	
	$xmlstr = @stream_get_contents($stream);
	@fclose($stream);
	
	$parser = xml_parser_create('');
	xml_parser_set_option($parser, XML_OPTION_TARGET_ENCODING, "UTF-8");
	xml_parser_set_option($parser, XML_OPTION_CASE_FOLDING, 0);
	xml_parser_set_option($parser, XML_OPTION_SKIP_WHITE, 1);
	xml_parse_into_struct($parser, trim($xmlstr), $xml_values);
	xml_parser_free($parser);
	
	if(!$xml_values) return;
	
	$xml_array = array();
	$parents = array();
	$opened_tags = array();
	$arr = array();
	$current = & $xml_array;
	$repeated_tag_index = array();
	
	foreach($xml_values as $data)
	{
		unset($attributes, $value);
		extract($data);
		$result = array();
		$attributes_data = array();
	
		if(isset($value))
		{
			if($priority == 'tag') $result = $value;
			else $result['value'] = $value;
		}
	
		if(isset($attributes) && $get_attributes)
		{
			foreach($attributes as $attr => $val)
			{
				if($priority == 'tag') $attributes_data[$attr] = $val;
				else $result['attr'][$attr] = $val;
			}
		}
	
		if($type == "open")
		{
			$parent[$level - 1] =& $current;
	
			if(!is_array($current) || (!in_array($tag, array_keys($current))))
			{
				$current[$tag] = $result;
				if($attributes_data) $current[$tag.'_attr'] = $attributes_data;
				$repeated_tag_index[$tag.'_'.$level] = 1;
				$current =& $current[$tag];
			}
			else
			{
				if(isset($current[$tag][0]))
				{
					$current[$tag][$repeated_tag_index[$tag.'_'.$level]] = $result;
					$repeated_tag_index[$tag.'_'.$level]++;
				}
				else
				{
					$current[$tag] = array($current[$tag],$result);
					$repeated_tag_index[$tag.'_'.$level] = 2;
				
					if(isset($current[$tag.'_attr']))
					{
						$current[$tag]['w_attr'] = $current[$tag.'_attr'];
						unset($current[$tag.'_attr']);
					}
				}
				$last_item_index = $repeated_tag_index[$tag.'_'.$level] - 1;
				$current =& $current[$tag][$last_item_index];
			}
		}
		else if($type == "complete")
		{
			if(!isset($current[$tag]))
			{
				$current[$tag] = $result;
				$repeated_tag_index[$tag.'_'.$level] = 1;
				if($priority == 'tag' && $attributes_data) $current[$tag.'_attr'] = $attributes_data;
			}
			else
			{
				if(isset($current[$tag][0]) && is_array($current[$tag]))
				{
					$current[$tag][$repeated_tag_index[$tag.'_'.$level]] = $result;
					if($priority == 'tag' && $get_attributes && $attributes_data) $current[$tag][$repeated_tag_index[$tag.'_'.$level].'_attr'] = $attributes_data;
					$repeated_tag_index[$tag.'_'.$level]++;
				}
				else
				{
					$current[$tag] = array($current[$tag], $result);
					$repeated_tag_index[$tag.'_'.$level] = 1;
		
					if($priority == 'tag' && $get_attributes)
					{
						if(isset($current[$tag.'_attr']))
						{
							$current[$tag]['w_attr'] = $current[$tag.'_attr'];
							unset($current[$tag.'_attr']);
						}
		
						if($attributes_data) $current[$tag][$repeated_tag_index[$tag.'_'.$level].'_attr'] = $attributes_data;
					}
					$repeated_tag_index[$tag.'_'.$level]++;
				}
			}
		}
		else if($type == 'close')
		{
			$current =& $parent[$level - 1];
		}
	}
	
	return $xml_array;
}

?>
