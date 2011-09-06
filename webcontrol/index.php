<?php
	include("utils.php");	
	makeHeader();
?>

<div id="content">


<b>Current display wall contents:</b>
<br>
<img src="http://symphony.waikato.ac.nz/display/wall.php?dn=1&width=210&height=525">
<img src="http://symphony.waikato.ac.nz/display/wall.php?dn=2&width=210&height=525">
<img src="http://symphony.waikato.ac.nz/display/wall.php?dn=3&width=210&height=525">
<img src="http://symphony.waikato.ac.nz/display/wall.php?dn=4&width=210&height=525">
<img src="http://symphony.waikato.ac.nz/display/wall.php?dn=5&width=210&height=525">

<br><br>

<b>Available applications:</b>
<table cellpadding="5">

<?php
	$request = api("list_apps");
	
	foreach ($request as $app) {
		echo "<tr>";
		echo "<td><b><a href='configure.php?name=".urlencode($app->name)."'>".$app->name."</a></b></td>";
		echo "<td>".$app->descr."</td>";
		echo "</tr>";
	}
?>

</table>

</div>
</body>
</html>
