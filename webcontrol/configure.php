<?php
	include("utils.php");	
	makeHeader();
?>

<div id="content">

<?php
	$request = api("app_details", array("name" => $_REQUEST["name"]));
	
	echo "<b>Configuring ".$request->name."</b><br>\n";
	echo "<i>".$request->descr."</i><br><br>\n";
	echo "<form action=\"invoke.php\" method=\"get\">\n";
	echo "<input type=\"hidden\" name=\"op\" value=\"run\">";
	echo "<input type=\"hidden\" name=\"app\" value=\"".$request->name."\">";
	echo "<table cellpadding=\"5\">\n";
	
	foreach($request->options as $option){
		echo "<tr>\n";
		echo "<td><b>".$option[0]."</b></td>\n";
		echo "<td>".$option[1]."</td>\n";
		echo "<td><input type=\"".$option[2]."\" name=\"".$option[0]."\" class=\"input_".$option[2]."\"></td>\n";
		echo "</tr>\n";				
	}
	
	echo "</table>\n";
	
	echo "<br><br>";
	echo "<input type=\"submit\" value=\"Launch\"> or ";
	echo "<a href=\"index.php\">Cancel</a> ";
	echo "</form>\n";
	
?>



</div>
</body>
</html>
