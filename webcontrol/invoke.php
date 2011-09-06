<?php

//This basically just proxies the request through php off to the actual API

include("utils.php");

if(!isset($_REQUEST["op"])){
	die("No request!");
}

api($_REQUEST["op"], $_REQUEST);

header("Location: index.php");

?>
