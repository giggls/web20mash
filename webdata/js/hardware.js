function genhtml_from_list(mylist) {
  var len = mylist.length;
  var text='';
  if (len >0) {
    for (var i = 0; i < len-1; i++) {
      text=text+mylist[i]+"<br />";
    }
    text=text+mylist[i];
  }
  return(text);
}

function gen_status_html(code) {
  var font;
  if (code==0) {
    font='<span style="color:green; font-weight:bold;">';
  } else {
    font='<span style="color:red; font-weight:bold;">';
  }
  return(font+device_error_messages[code]+'</span>');
}

$(document).ready(function () {
  $.ajax({ 
    type: 'GET', 
    url: '/getdevinfo', 
    dataType: 'json',
    success: function (data) {
	
	$("#sensor_name").text(data[0].name);
	$("#sensor_device").text(data[0].device);
	$("#sensor_status").html(gen_status_html(data[0].error));
	$("#sensor_devlist").text(data[0].devlist);
	$("#sensor_options").text(data[0].options);

        $("#act0_name").text(data[1].name);
        $("#act0_device").text(data[1].device);
        $("#act0_status").html(gen_status_html(data[1].error));
        $("#act0_devlist").html(genhtml_from_list(data[1].devlist));
        $("#act0_options").html(genhtml_from_list(data[1].options));
	if (data.length==3) {
          $('#stirrer').show();
          $("#act1_name").text(data[2].name);
          $("#act1_device").text(data[2].device);
          $("#act1_status").html(gen_status_html(data[2].error));   
          $("#act1_devlist").html(genhtml_from_list(data[2].devlist));
          $("#act1_options").html(genhtml_from_list(data[2].options));
	};
    }
  });
});
