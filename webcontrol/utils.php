<?php


function api($op, $params = array()){
	$url = api_link($op, $params);
	return api_get($url);
}

function api_link($op, $params = array()){

	//Configure this!
	$API_ENDPOINT = "http://localhost:8080/";
	
	$url = $API_ENDPOINT . "?op=".urlencode($op);
	foreach ($params as $key => $value) {
		//so that we can just pass $_REQUEST
		if(strcmp($key, "op") != 0){
			$url = $url."&".urlencode($key)."=".urlencode($value);
		}
	}
	return $url;
}

function api_get($url){

	$data = file_get_contents($url);
		
	if($data === FALSE){
		die("(Could not contact webcontrol API (<a href='".$url."'>request</a>)");
	}	
	
	return json_decode($data);
}



function makeHeader(){

?>	
<html>
<head>
<title>Symphony - ClusterGL WebControl</title>
<link type="text/css" rel="stylesheet" href="css/main.css" />
</head>
<body>
<div id="header">
<b>ClusterGL Status:</b> 

<?php
	$request = api("status");	
	echo $request->status;
?>

 ( <a href="invoke.php?op=stop">stop</a> )


<div id="rightheader">
<a href="http://code.google.com/p/clustergl2">ClusterGL</a> and WebControl made by <a href="emailto:paul@bieh.net">Paul Hunkin</a><br>
However, all questions should be directed to <a href="emailto:dmneal@wand.net.nz">Donald Neal</a>
</div>

</div>

<?php

}

?>
