function define_key(keycode, caption, x, y){
	var html = "";

	html += "<input value=\"" + caption + "\"" +
		"type=\"button\"" +
		"onmousedown=\"keydown(" + keycode + ")\"" +
		"onmouseup=\"keyup(" + keycode + ")\"" +
		"class=\"btn btn-large btn-primary\" style=\"" +
		"position:absolute; " +
		"left: " + x + "; " +
		"top: " + y + "; " +
		"height: 128px; " +
		"width: 128px\">" +
		"</input>";

	$("#keyboard").append(html);
}

function keydown(keycode){
	$.ajax({url:"/kbdown/" + keycode});
}

function keyup(keycode){
	$.ajax({url:"/kbup/" + keycode});
}